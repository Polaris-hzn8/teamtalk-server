/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: RouteServConn.cpp
 Update Time: Thu 15 Jun 2023 00:57:17 CST
 brief:
*/

#include <teamtalk/imcore/common/tools.h>
#include <teamtalk/imcore/slog/slog.h>
#include <teamtalk/imcore/ttidl/service.pb.h>
#include <teamtalk/imcore/ttidl/buddy.pb.h>
#include <teamtalk/imcore/ttidl/file.pb.h>
#include <teamtalk/imcore/ttidl/message.pb.h>
#include <teamtalk/imcore/ttidl/other.pb.h>
#include <teamtalk/imcore/ttidl/switch_service.pb.h>

#include "connection/attach_data.h"
#include "connection/msg_conn.h"
#include "connection/db_serv_conn.h"
#include "connection/file_serv_conn.h"
#include "connection/push_serv_conn.h"
#include "connection/login_serv_conn.h"
#include "connection/route_serv_conn.h"

#include "domain/user/im_user.h"
#include "service/chat_handler/group_chat.h"
#include "service/file_handler/file_handler.h"

namespace teamtalk::msg_server::connection {

using namespace std;

namespace ttuser = teamtalk::msg_server::domain::user;
namespace ttchat_handler = teamtalk::msg_server::service::chat_handler;
namespace ttfile_handler = teamtalk::msg_server::service::file_handler;
namespace ttidlbase = teamtalk::imcore::ttidl::base_define;
namespace ttidlserver = teamtalk::imcore::ttidl::service;
namespace ttidlother = teamtalk::imcore::ttidl::other;
namespace ttidlmessage = teamtalk::imcore::ttidl::message;
namespace ttidlbuddy = teamtalk::imcore::ttidl::buddy;
namespace ttidlfile = teamtalk::imcore::ttidl::file;
namespace ttidlswitchservice = teamtalk::imcore::ttidl::switch_service;
namespace ttcommon = teamtalk::imcore::common;

static ttnetlib::ConnMap_t g_route_server_conn_map;

static ttserverinfo::serv_info_t* g_route_server_list;
static uint32_t g_route_server_count;
static CRouteServConn* g_master_rs_conn = NULL;
static ttfile_handler::CFileHandler* s_file_handler = NULL;
static ttchat_handler::CGroupChat* s_group_chat = NULL;

void route_server_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam) {
  ttnetlib::ConnMap_t::iterator it_old;
  CRouteServConn* pConn = NULL;
  uint64_t cur_time = ttcommon::get_tick_count();

  for (ttnetlib::ConnMap_t::iterator it = g_route_server_conn_map.begin(); it != g_route_server_conn_map.end();) {
    it_old = it;
    it++;

    pConn = (CRouteServConn*)it_old->second;
    pConn->OnTimer(cur_time);
  }

  // reconnect RouteServer
  ttserverinfo::serv_check_reconnect<CRouteServConn>(g_route_server_list, g_route_server_count);
}

void init_route_serv_conn(ttserverinfo::serv_info_t* server_list, uint32_t server_count) {
  g_route_server_list = server_list;
  g_route_server_count = server_count;

  ttserverinfo::serv_init<CRouteServConn>(g_route_server_list, g_route_server_count);

  ttnetlib::netlib_register_timer(route_server_conn_timer_callback, NULL, 1000);
  s_file_handler = ttfile_handler::CFileHandler::getInstance();
  s_group_chat = ttchat_handler::CGroupChat::GetInstance();
}

bool is_route_server_available() {
  CRouteServConn* pConn = NULL;

  for (uint32_t i = 0; i < g_route_server_count; i++) {
    pConn = (CRouteServConn*)g_route_server_list[i].serv_conn;
    if (pConn && pConn->IsOpen()) {
      return true;
    }
  }

  return false;
}

void send_to_all_route_server(ttnetlib::CImPdu* pPdu) {
  CRouteServConn* pConn = NULL;

  for (uint32_t i = 0; i < g_route_server_count; i++) {
    pConn = (CRouteServConn*)g_route_server_list[i].serv_conn;
    if (pConn && pConn->IsOpen()) {
      pConn->SendPdu(pPdu);
    }
  }
}

// get the oldest route server connection
CRouteServConn* get_route_serv_conn() {
  return g_master_rs_conn;
}

void update_master_route_serv_conn() {
  uint64_t oldest_connect_time = (uint64_t)-1;
  CRouteServConn* pOldestConn = NULL;

  CRouteServConn* pConn = NULL;

  for (uint32_t i = 0; i < g_route_server_count; i++) {
    pConn = (CRouteServConn*)g_route_server_list[i].serv_conn;
    if (pConn && pConn->IsOpen() && (pConn->GetConnectTime() < oldest_connect_time)) {
      pOldestConn = pConn;
      oldest_connect_time = pConn->GetConnectTime();
    }
  }

  g_master_rs_conn = pOldestConn;

  if (g_master_rs_conn) {
    ttidlserver::IMRoleSet msg;
    msg.set_master(1);
    ttnetlib::CImPdu pdu;
    pdu.SetPBMsg(&msg);
    pdu.SetServiceId(ttidlbase::SID_OTHER);
    pdu.SetCommandId(ttidlbase::CID_OTHER_ROLE_SET);
    g_master_rs_conn->SendPdu(&pdu);
  }
}

CRouteServConn::CRouteServConn() {
  m_bOpen = false;
  m_serv_idx = 0;
}

CRouteServConn::~CRouteServConn() {}

void CRouteServConn::Connect(const char* server_ip, uint16_t server_port, uint32_t idx) {
  log_info("Connecting to RouteServer %s:%d ", server_ip, server_port);

  m_serv_idx = idx;
  m_handle = ttnetlib::netlib_connect(server_ip, server_port, ttnetlib::imconn_callback, (void*)&g_route_server_conn_map);

  if (m_handle != NETLIB_INVALID_HANDLE) {
    g_route_server_conn_map.insert(make_pair(m_handle, this));
  }
}

void CRouteServConn::Close() {
  ttserverinfo::serv_reset<CRouteServConn>(g_route_server_list, g_route_server_count, m_serv_idx);

  m_bOpen = false;
  if (m_handle != NETLIB_INVALID_HANDLE) {
    ttnetlib::netlib_close(m_handle);
    g_route_server_conn_map.erase(m_handle);
  }

  ReleaseRef();

  if (g_master_rs_conn == this) {
    update_master_route_serv_conn();
  }
}

void CRouteServConn::OnConfirm() {
  log_info("connect to route server success ");
  m_bOpen = true;
  m_connect_time = ttcommon::get_tick_count();
  g_route_server_list[m_serv_idx].reconnect_cnt = MIN_RECONNECT_CNT / 2;

  if (g_master_rs_conn == NULL) {
    update_master_route_serv_conn();
  }

  list<user_stat_t> online_user_list;
  ttuser::CImUserManager::GetInstance()->GetOnlineUserInfo(&online_user_list);
  ttidlserver::IMOnlineUserInfo msg;
  for (list<user_stat_t>::iterator it = online_user_list.begin(); it != online_user_list.end(); it++) {
    user_stat_t user_stat = *it;
    ttidlbase::ServerUserStat* server_user_stat = msg.add_user_stat_list();
    server_user_stat->set_user_id(user_stat.user_id);
    server_user_stat->set_status((ttidlbase::UserStatType)user_stat.status);
    server_user_stat->set_client_type((ttidlbase::ClientType)user_stat.client_type);
  }
  ttnetlib::CImPdu pdu;
  pdu.SetPBMsg(&msg);
  pdu.SetServiceId(ttidlbase::SID_OTHER);
  pdu.SetCommandId(ttidlbase::CID_OTHER_ONLINE_USER_INFO);
  SendPdu(&pdu);
}

void CRouteServConn::OnClose() {
  log_info("onclose from route server handle=%d ", m_handle);
  Close();
}

void CRouteServConn::OnTimer(uint64_t curr_tick) {
  if (curr_tick > m_last_send_tick + SERVER_HEARTBEAT_INTERVAL) {
    ttidlother::IMHeartBeat msg;
    ttnetlib::CImPdu pdu;
    pdu.SetPBMsg(&msg);
    pdu.SetServiceId(ttidlbase::SID_OTHER);
    pdu.SetCommandId(ttidlbase::CID_OTHER_HEARTBEAT);
    SendPdu(&pdu);
  }

  if (curr_tick > m_last_recv_tick + SERVER_TIMEOUT) {
    log_info("conn to route server timeout ");
    Close();
  }
}

void CRouteServConn::HandlePdu(ttnetlib::CImPdu* pPdu) {
  switch (pPdu->GetCommandId()) {
    case ttidlbase::CID_OTHER_HEARTBEAT:
      break;
    case ttidlbase::CID_OTHER_SERVER_KICK_USER:
      _HandleKickUser(pPdu);
      break;
    case ttidlbase::CID_BUDDY_LIST_STATUS_NOTIFY:
      _HandleStatusNotify(pPdu);
      break;
    case ttidlbase::CID_BUDDY_LIST_USERS_STATUS_RESPONSE:
      _HandleUsersStatusResponse(pPdu);
      break;
    case ttidlbase::CID_MSG_READ_NOTIFY:
      _HandleMsgReadNotify(pPdu);
      break;
    case ttidlbase::CID_MSG_DATA:
      _HandleMsgData(pPdu);
      break;
    case ttidlbase::CID_SWITCH_P2P_CMD:
      _HandleP2PMsg(pPdu);
      break;
    case ttidlbase::CID_OTHER_LOGIN_STATUS_NOTIFY:
      _HandlePCLoginStatusNotify(pPdu);
      break;
    case ttidlbase::CID_BUDDY_LIST_REMOVE_SESSION_NOTIFY:
      _HandleRemoveSessionNotify(pPdu);
      break;
    case ttidlbase::CID_BUDDY_LIST_SIGN_INFO_CHANGED_NOTIFY:
      _HandleSignInfoChangedNotify(pPdu);
      break;
    case ttidlbase::CID_GROUP_CHANGE_MEMBER_NOTIFY:
      s_group_chat->HandleGroupChangeMemberBroadcast(pPdu);
      break;
    case ttidlbase::CID_FILE_NOTIFY:
      s_file_handler->HandleFileNotify(pPdu);
      break;
    default:
      log_info("unknown cmd id=%d ", pPdu->GetCommandId());
      break;
  }
}

void CRouteServConn::_HandleKickUser(ttnetlib::CImPdu* pPdu) {
  ttidlserver::IMServerKickUser msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  uint32_t user_id = msg.user_id();
  uint32_t client_type = msg.client_type();
  uint32_t reason = msg.reason();
  log_info("HandleKickUser, user_id=%u, client_type=%u, reason=%u. ", user_id, client_type, reason);

  ttuser::CImUser* pUser = ttuser::CImUserManager::GetInstance()->GetImUserById(user_id);
  if (pUser) {
    pUser->KickOutSameClientType(client_type, reason);
  }
}

// friend online/off-line notify
void CRouteServConn::_HandleStatusNotify(ttnetlib::CImPdu* pPdu) {
  ttidlbuddy::IMUserStatNotify msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  ttidlbase::UserStat user_stat = msg.user_stat();

  log_info("HandleFriendStatusNotify, user_id=%u, status=%u ", user_stat.user_id(), user_stat.status());

  // send friend online message to client
  ttuser::CImUserManager::GetInstance()->BroadcastPdu(pPdu, CLIENT_TYPE_FLAG_PC);
}

void CRouteServConn::_HandleMsgData(ttnetlib::CImPdu* pPdu) {
  ttidlmessage::IMMsgData msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
  if (CHECK_MSG_TYPE_GROUP(msg.msg_type())) {
    s_group_chat->HandleGroupMessageBroadcast(pPdu);
    return;
  }
  uint32_t from_user_id = msg.from_user_id();
  uint32_t to_user_id = msg.to_session_id();
  uint32_t msg_id = msg.msg_id();
  log_info("HandleMsgData, %u->%u, msg_id=%u. ", from_user_id, to_user_id, msg_id);

  ttuser::CImUser* pFromImUser = ttuser::CImUserManager::GetInstance()->GetImUserById(from_user_id);
  if (pFromImUser) {
    pFromImUser->BroadcastClientMsgData(pPdu, msg_id, NULL, from_user_id);
  }

  ttuser::CImUser* pToImUser = ttuser::CImUserManager::GetInstance()->GetImUserById(to_user_id);
  if (pToImUser) {
    pToImUser->BroadcastClientMsgData(pPdu, msg_id, NULL, from_user_id);
  }
}

void CRouteServConn::_HandleMsgReadNotify(ttnetlib::CImPdu* pPdu) {
  ttidlmessage::IMMsgDataReadNotify msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  uint32_t req_id = msg.user_id();
  uint32_t session_id = msg.session_id();
  uint32_t msg_id = msg.msg_id();
  uint32_t session_type = msg.session_type();

  log_info(
    "HandleMsgReadNotify, user_id=%u, session_id=%u, session_type=%u, "
    "msg_id=%u. ",
    req_id,
    session_id,
    session_type,
    msg_id);
  ttuser::CImUser* pUser = ttuser::CImUserManager::GetInstance()->GetImUserById(req_id);
  if (pUser) {
    pUser->BroadcastPdu(pPdu);
  }
}

void CRouteServConn::_HandleP2PMsg(ttnetlib::CImPdu* pPdu) {
  ttidlswitchservice::IMP2PCmdMsg msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  uint32_t from_user_id = msg.from_user_id();
  uint32_t to_user_id = msg.to_user_id();

  log_info("HandleP2PMsg, %u->%u ", from_user_id, to_user_id);

  ttuser::CImUser* pFromImUser = ttuser::CImUserManager::GetInstance()->GetImUserById(from_user_id);
  ttuser::CImUser* pToImUser = ttuser::CImUserManager::GetInstance()->GetImUserById(to_user_id);

  if (pFromImUser) {
    pFromImUser->BroadcastPdu(pPdu);
  }

  if (pToImUser) {
    pToImUser->BroadcastPdu(pPdu);
  }
}

void CRouteServConn::_HandleUsersStatusResponse(ttnetlib::CImPdu* pPdu) {
  ttidlbuddy::IMUsersStatRsp msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  uint32_t user_id = msg.user_id();
  uint32_t result_count = msg.user_stat_list_size();
  log_info("HandleUsersStatusResp, user_id=%u, query_count=%u ", user_id, result_count);

  CPduAttachData attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
  if (attach_data.GetType() == ATTACH_TYPE_HANDLE) {
    uint32_t handle = attach_data.GetHandle();
    CMsgConn* pConn = ttuser::CImUserManager::GetInstance()->GetMsgConnByHandle(user_id, handle);
    if (pConn) {
      msg.clear_attach_data();
      pPdu->SetPBMsg(&msg);
      pConn->SendPdu(pPdu);
    }
  } else if (attach_data.GetType() == ATTACH_TYPE_PDU_FOR_PUSH) {
    ttidlbase::UserStat user_stat = msg.user_stat_list(0);
    ttidlserver::IMPushToUserReq msg2;
    CHECK_PB_PARSE_MSG(msg2.ParseFromArray(attach_data.GetPdu(), attach_data.GetPduLength()));
    ttidlbase::UserTokenInfo* user_token = msg2.mutable_user_token_list(0);

    // pc client登录，则为勿打扰式推送
    if (user_stat.status() == ttidlbase::USER_STATUS_ONLINE) {
      user_token->set_push_type(IM_PUSH_TYPE_SILENT);
      log_info("HandleUsersStatusResponse, user id: %d, push type: normal. ", user_stat.user_id());
    } else {
      user_token->set_push_type(IM_PUSH_TYPE_NORMAL);
      log_info("HandleUsersStatusResponse, user id: %d, push type: normal. ", user_stat.user_id());
    }
    ttnetlib::CImPdu pdu;
    pdu.SetPBMsg(&msg2);
    pdu.SetServiceId(ttidlbase::SID_OTHER);
    pdu.SetCommandId(ttidlbase::CID_OTHER_PUSH_TO_USER_REQ);

    CPushServConn* PushConn = get_push_serv_conn();
    if (PushConn) {
      PushConn->SendPdu(&pdu);
    }
  } else if (attach_data.GetType() == ATTACH_TYPE_HANDLE_AND_PDU_FOR_FILE) {
    ttidlbase::UserStat user_stat = msg.user_stat_list(0);
    ttidlserver::IMFileTransferReq msg3;
    CHECK_PB_PARSE_MSG(msg3.ParseFromArray(attach_data.GetPdu(), attach_data.GetPduLength()));
    uint32_t handle = attach_data.GetHandle();

    ttidlbase::TransferFileType trans_mode = ttidlbase::FILE_TYPE_OFFLINE;
    if (user_stat.status() == ttidlbase::USER_STATUS_ONLINE) {
      trans_mode = ttidlbase::FILE_TYPE_ONLINE;
    }
    msg3.set_trans_mode(trans_mode);
    ttnetlib::CImPdu pdu;
    pdu.SetPBMsg(&msg3);
    pdu.SetServiceId(ttidlbase::SID_OTHER);
    pdu.SetCommandId(ttidlbase::CID_OTHER_FILE_TRANSFER_REQ);
    pdu.SetSeqNum(pPdu->GetSeqNum());
    CFileServConn* pConn = get_random_file_serv_conn();
    if (pConn) {
      pConn->SendPdu(&pdu);
    } else {
      log_info("no file server ");
      ttidlfile::IMFileRsp msg4;
      msg4.set_result_code(1);
      msg4.set_from_user_id(msg3.from_user_id());
      msg4.set_to_user_id(msg3.to_user_id());
      msg4.set_file_name(msg3.file_name());
      msg4.set_task_id("");
      msg4.set_trans_mode(msg3.trans_mode());
      ttnetlib::CImPdu pdu2;
      pdu2.SetPBMsg(&msg4);
      pdu2.SetServiceId(ttidlbase::SID_FILE);
      pdu2.SetCommandId(ttidlbase::CID_FILE_RESPONSE);
      pdu2.SetSeqNum(pPdu->GetSeqNum());
      CMsgConn* pMsgConn = ttuser::CImUserManager::GetInstance()->GetMsgConnByHandle(msg3.from_user_id(), handle);
      if (pMsgConn) {
        pMsgConn->SendPdu(&pdu2);
      }
    }
  }
}

void CRouteServConn::_HandleRemoveSessionNotify(ttnetlib::CImPdu* pPdu) {
  ttidlbuddy::IMRemoveSessionNotify msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  uint32_t user_id = msg.user_id();
  uint32_t session_id = msg.session_id();
  log_info("HandleRemoveSessionNotify, user_id=%u, session_id=%u ", user_id, session_id);
  ttuser::CImUser* pUser = ttuser::CImUserManager::GetInstance()->GetImUserById(user_id);
  if (pUser) {
    pUser->BroadcastPdu(pPdu);
  }
}

void CRouteServConn::_HandlePCLoginStatusNotify(ttnetlib::CImPdu* pPdu) {
  ttidlserver::IMServerPCLoginStatusNotify msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  uint32_t user_id = msg.user_id();
  uint32_t login_status = msg.login_status();
  log_info("HandlePCLoginStatusNotify, user_id=%u, login_status=%u ", user_id, login_status);

  ttuser::CImUser* pUser = ttuser::CImUserManager::GetInstance()->GetImUserById(user_id);
  if (pUser) {
    pUser->SetPCLoginStatus(login_status);
    ttidlbuddy::IMPCLoginStatusNotify msg2;
    msg2.set_user_id(user_id);
    if (IM_PC_LOGIN_STATUS_ON == login_status) {
      msg2.set_login_stat(ttidlbase::USER_STATUS_ONLINE);
    } else {
      msg2.set_login_stat(ttidlbase::USER_STATUS_OFFLINE);
    }
    ttnetlib::CImPdu pdu;
    pdu.SetPBMsg(&msg2);
    pdu.SetServiceId(ttidlbase::SID_BUDDY_LIST);
    pdu.SetCommandId(ttidlbase::CID_BUDDY_LIST_PC_LOGIN_STATUS_NOTIFY);
    pUser->BroadcastPduToMobile(&pdu);
  }
}

void CRouteServConn::_HandleSignInfoChangedNotify(ttnetlib::CImPdu* pPdu) {
  ttidlbuddy::IMSignInfoChangedNotify msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  log_info(
    "HandleSignInfoChangedNotify, changed_user_id=%u, sign_info=%s ", msg.changed_user_id(), msg.sign_info().c_str());

  // send friend online message to client
  ttuser::CImUserManager::GetInstance()->BroadcastPdu(pPdu, CLIENT_TYPE_FLAG_BOTH);
}

}  // namespace teamtalk::msg_server::connection