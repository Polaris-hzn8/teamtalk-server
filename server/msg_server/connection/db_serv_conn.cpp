/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: DBServConn.cpp
 Update Time: Thu 15 Jun 2023 00:54:24 CST
 brief:
*/

#include <json/json.h>
#include <teamtalk/imcore/common/tools.h>
#include <teamtalk/imcore/security/security.h>
#include <teamtalk/imcore/slog/slog.h>
#include <teamtalk/imcore/netlib/core/im_pdu.h>

#include <teamtalk/imcore/ttidl/buddy.pb.h>
#include <teamtalk/imcore/ttidl/login.pb.h>
#include <teamtalk/imcore/ttidl/message.pb.h>
#include <teamtalk/imcore/ttidl/other.pb.h>
#include <teamtalk/imcore/ttidl/service.pb.h>

#include <teamtalk/sbase/global_define.h>

#include "connection/msg_conn.h"
#include "connection/attach_data.h"
#include "connection/db_serv_conn.h"
#include "connection/push_serv_conn.h"
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
namespace ttidllogin = teamtalk::imcore::ttidl::login;
namespace ttidlother = teamtalk::imcore::ttidl::other;
namespace ttidlmessage = teamtalk::imcore::ttidl::message;
namespace ttidlbuddy = teamtalk::imcore::ttidl::buddy;
namespace ttsecurity = teamtalk::imcore::security;
namespace ttcommon = teamtalk::imcore::common;

static ttnetlib::ConnMap_t g_db_server_conn_map;

static ttserverinfo::serv_info_t* g_db_server_list = NULL;
static uint32_t g_db_server_count = 0;        // 到DBServer的总连接数
static uint32_t g_db_server_login_count = 0;  // 到进行登录处理的DBServer的总连接数
static ttchat_handler::CGroupChat* s_group_chat = NULL;
static ttfile_handler::CFileHandler* s_file_handler = NULL;

static void db_server_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam) {
  ttnetlib::ConnMap_t::iterator it_old;
  CDBServConn* pConn = NULL;
  uint64_t cur_time = ttcommon::get_tick_count();

  for (ttnetlib::ConnMap_t::iterator it = g_db_server_conn_map.begin(); it != g_db_server_conn_map.end();) {
    it_old = it;
    it++;

    pConn = (CDBServConn*)it_old->second;
    if (pConn->IsOpen()) {
      pConn->OnTimer(cur_time);
    }
  }

  // reconnect DB Storage Server
  // will reconnect in 4s, 8s, 16s, 32s, 64s, 4s 8s ...
  ttserverinfo::serv_check_reconnect<CDBServConn>(g_db_server_list, g_db_server_count);
}

void init_db_serv_conn(ttserverinfo::serv_info_t* server_list, uint32_t server_count, uint32_t concur_conn_cnt) {
  g_db_server_list = server_list;
  g_db_server_count = server_count;

  uint32_t total_db_instance = server_count / concur_conn_cnt;
  g_db_server_login_count = (total_db_instance / 2) * concur_conn_cnt;
  log_info(
    "DB server connection index for login business: [0, %u), for other "
    "business: [%u, %u) ",
    g_db_server_login_count,
    g_db_server_login_count,
    g_db_server_count);

  ttserverinfo::serv_init<CDBServConn>(g_db_server_list, g_db_server_count);

  ttnetlib::netlib_register_timer(db_server_conn_timer_callback, NULL, 1000);
  s_group_chat = ttchat_handler::CGroupChat::GetInstance();
  s_file_handler = ttfile_handler::CFileHandler::getInstance();
}

// get a random db server connection in the range [start_pos, stop_pos)
static CDBServConn* get_db_server_conn_in_range(uint32_t start_pos, uint32_t stop_pos) {
  uint32_t i = 0;
  CDBServConn* pDbConn = NULL;

  // determine if there is a valid DB server connection
  for (i = start_pos; i < stop_pos; i++) {
    pDbConn = (CDBServConn*)g_db_server_list[i].serv_conn;
    if (pDbConn && pDbConn->IsOpen()) {
      break;
    }
  }

  // no valid DB server connection
  if (i == stop_pos) {
    return NULL;
  }

  // return a random valid DB server connection
  while (true) {
    int i = rand() % (stop_pos - start_pos) + start_pos;
    pDbConn = (CDBServConn*)g_db_server_list[i].serv_conn;
    if (pDbConn && pDbConn->IsOpen()) {
      break;
    }
  }

  return pDbConn;
}

CDBServConn* get_db_serv_conn_for_login() {
  // 先获取login业务的实例，没有就去获取其他业务流程的实例
  CDBServConn* pDBConn = get_db_server_conn_in_range(0, g_db_server_login_count);
  if (!pDBConn) {
    pDBConn = get_db_server_conn_in_range(g_db_server_login_count, g_db_server_count);
  }

  return pDBConn;
}

CDBServConn* get_db_serv_conn() {
  // 先获取其他业务流程的实例，没有就去获取login业务的实例
  CDBServConn* pDBConn = get_db_server_conn_in_range(g_db_server_login_count, g_db_server_count);
  if (!pDBConn) {
    pDBConn = get_db_server_conn_in_range(0, g_db_server_login_count);
  }

  return pDBConn;
}

CDBServConn::CDBServConn() {
  m_bOpen = false;
}

CDBServConn::~CDBServConn() {}

void CDBServConn::Connect(const char* server_ip, uint16_t server_port, uint32_t serv_idx) {
  log_info("Connecting to DB Storage Server %s:%d ", server_ip, server_port);

  m_serv_idx = serv_idx;
  m_handle = ttnetlib::netlib_connect(server_ip, server_port, ttnetlib::imconn_callback, (void*)&g_db_server_conn_map);

  if (m_handle != NETLIB_INVALID_HANDLE) {
    g_db_server_conn_map.insert(make_pair(m_handle, this));
  }
}

void CDBServConn::Close() {
  // reset server information for the next connect
  ttserverinfo::serv_reset<CDBServConn>(g_db_server_list, g_db_server_count, m_serv_idx);

  if (m_handle != NETLIB_INVALID_HANDLE) {
    ttnetlib::netlib_close(m_handle);
    g_db_server_conn_map.erase(m_handle);
  }

  ReleaseRef();
}

void CDBServConn::OnConfirm() {
  log_info("connect to db server success");
  m_bOpen = true;
  g_db_server_list[m_serv_idx].reconnect_cnt = MIN_RECONNECT_CNT / 2;
}

void CDBServConn::OnClose() {
  log_info("onclose from db server handle=%d", m_handle);
  Close();
}

void CDBServConn::OnTimer(uint64_t curr_tick) {
  if (curr_tick > m_last_send_tick + SERVER_HEARTBEAT_INTERVAL) {
    ttidlother::IMHeartBeat msg;
    ttnetlib::CImPdu pdu;
    pdu.SetPBMsg(&msg);
    pdu.SetServiceId(ttidlbase::SID_OTHER);
    pdu.SetCommandId(ttidlbase::CID_OTHER_HEARTBEAT);
    SendPdu(&pdu);
  }

  if (curr_tick > m_last_recv_tick + SERVER_TIMEOUT) {
    log_info("conn to db server timeout");
    Close();
  }
}

void CDBServConn::HandlePdu(ttnetlib::CImPdu* pPdu) {
  switch (pPdu->GetCommandId()) {
    case ttidlbase::CID_OTHER_HEARTBEAT:
      break;
    case ttidlbase::CID_OTHER_VALIDATE_RSP:
      _HandleValidateResponse(pPdu);
      break;
    case ttidlbase::CID_LOGIN_RES_DEVICETOKEN:
      _HandleSetDeviceTokenResponse(pPdu);
      break;
    case ttidlbase::CID_LOGIN_RES_PUSH_SHIELD:
      _HandlePushShieldResponse(pPdu);
      break;
    case ttidlbase::CID_LOGIN_RES_QUERY_PUSH_SHIELD:
      _HandleQueryPushShieldResponse(pPdu);
      break;
    case ttidlbase::CID_MSG_UNREAD_CNT_RESPONSE:
      _HandleUnreadMsgCountResponse(pPdu);
      break;
    case ttidlbase::CID_MSG_LIST_RESPONSE:
      _HandleGetMsgListResponse(pPdu);
      break;
    case ttidlbase::CID_MSG_GET_BY_MSG_ID_RES:
      _HandleGetMsgByIdResponse(pPdu);
      break;
    case ttidlbase::CID_MSG_DATA:
      _HandleMsgData(pPdu);
      break;
    case ttidlbase::CID_MSG_GET_LATEST_MSG_ID_RSP:
      _HandleGetLatestMsgIDRsp(pPdu);
      break;
    case ttidlbase::CID_BUDDY_LIST_RECENT_CONTACT_SESSION_RESPONSE:
      _HandleRecentSessionResponse(pPdu);
      break;
    case ttidlbase::CID_BUDDY_LIST_ALL_USER_RESPONSE:
      _HandleAllUserResponse(pPdu);
      break;
    case ttidlbase::CID_BUDDY_LIST_USER_INFO_RESPONSE:
      _HandleUsersInfoResponse(pPdu);
      break;
    case ttidlbase::CID_BUDDY_LIST_REMOVE_SESSION_RES:
      _HandleRemoveSessionResponse(pPdu);
      break;
    case ttidlbase::CID_BUDDY_LIST_CHANGE_AVATAR_RESPONSE:
      _HandleChangeAvatarResponse(pPdu);
      break;
    case ttidlbase::CID_BUDDY_LIST_CHANGE_SIGN_INFO_RESPONSE:
      _HandleChangeSignInfoResponse(pPdu);
      break;
    case ttidlbase::CID_BUDDY_LIST_DEPARTMENT_RESPONSE:
      _HandleDepartmentResponse(pPdu);
      break;
    case ttidlbase::CID_OTHER_GET_DEVICE_TOKEN_RSP:
      _HandleGetDeviceTokenResponse(pPdu);
      break;
    case ttidlbase::CID_OTHER_GET_SHIELD_RSP:
      s_group_chat->HandleGroupGetShieldByGroupResponse(pPdu);
      break;
    case ttidlbase::CID_OTHER_STOP_RECV_PACKET:
      _HandleStopReceivePacket(pPdu);
      break;
    // group
    case ttidlbase::CID_GROUP_NORMAL_LIST_RESPONSE:
      s_group_chat->HandleGroupNormalResponse(pPdu);
      break;
    case ttidlbase::CID_GROUP_INFO_RESPONSE:
      s_group_chat->HandleGroupInfoResponse(pPdu);
      break;
    case ttidlbase::CID_GROUP_CREATE_RESPONSE:
      s_group_chat->HandleGroupCreateResponse(pPdu);
      break;
    case ttidlbase::CID_GROUP_CHANGE_MEMBER_RESPONSE:
      s_group_chat->HandleGroupChangeMemberResponse(pPdu);
      break;
    case ttidlbase::CID_GROUP_SHIELD_GROUP_RESPONSE:
      s_group_chat->HandleGroupShieldGroupResponse(pPdu);
      break;

    case ttidlbase::CID_FILE_HAS_OFFLINE_RES:
      s_file_handler->HandleFileHasOfflineRes(pPdu);
      break;

    default:
      log_info("db server, wrong cmd id=%d ", pPdu->GetCommandId());
  }
}

//处理验证响应消息的函数
void CDBServConn::_HandleValidateResponse(ttnetlib::CImPdu* pPdu) {
  ttidlserver::IMValidateRsp msg;
  // 1.解析验证响应消息 获取登录名 login_name、结果码 result 和 结果字符串
  // result_string
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
  string login_name = msg.user_name();
  uint32_t result = msg.result_code();
  string result_string = msg.result_string();

  // 2.根据附加数据构造 CDbAttachData 对象
  CDbAttachData attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
  log_info("HandleValidateResp, user_name=%s, result=%d", login_name.c_str(), result);

  // 3.根据登录名获取用户对象 pImUser 记录日志
  ttuser::CImUser* pImUser = ttuser::CImUserManager::GetInstance()->GetImUserByLoginName(login_name);
  CMsgConn* pMsgConn = NULL;
  if (!pImUser) {
    // 如果用户对象存在，则根据附加数据的句柄获取未验证的消息连接对象 pMsgConn
    log_info("ImUser for user_name=%s not exist", login_name.c_str());
    return;
  } else {
    // 如果 pMsgConn 不存在或者已经打开，则记录日志并返回
    pMsgConn = pImUser->GetUnValidateMsgConn(attach_data.GetHandle());
    if (!pMsgConn || pMsgConn->IsOpen()) {
      log_info("no such conn is validated, user_name=%s", login_name.c_str());
      return;
    }
  }

  // 4.如果结果码 result 不为零，将结果码重置为
  // ttidlbase::REFUSE_REASON_DB_VALIDATE_FAILED
  if (result != 0)
    result = ttidlbase::REFUSE_REASON_DB_VALIDATE_FAILED;

  // 5.如果结果码为零表示验证成功，继续处理验证成功的逻辑
  if (result == 0) {
    // 5-1.获取用户信息 user_info  user_id  用户对象pUser
    ttidlbase::UserInfo user_info = msg.user_info();
    uint32_t user_id = user_info.user_id();
    ttuser::CImUser* pUser = ttuser::CImUserManager::GetInstance()->GetImUserById(user_id);
    if (pUser) {
      // 已存在该ID的用户对象 pUser 则将该连接添加到 pUser
      // 的未验证连接列表中，并从 pImUser 的未验证连接列表中删除
      pUser->AddUnValidateMsgConn(pMsgConn);
      pImUser->DelUnValidateMsgConn(pMsgConn);
      // 如果 pImUser
      // 的未验证连接列表为空，表示没有其他未验证的连接了，可以移除该用户对象
      if (pImUser->IsMsgConnEmpty()) {
        ttuser::CImUserManager::GetInstance()->RemoveImUserByLoginName(login_name);
        delete pImUser;
      }
    } else {
      // 如果用户对象 pUser 不存在，则将 pUser 设置为 pImUser
      pUser = pImUser;
    }

    // 5-2.设置 pUser 的用户ID、昵称、验证状态
    pUser->SetUserId(user_id);                       // uid
    pUser->SetNickName(user_info.user_nick_name());  // nick_name
    pUser->SetValidated();                           // SetValidated
    ttuser::CImUserManager::GetInstance()->AddImUserById(user_id, pUser);

    // 5-3.根据连接的客户端类型踢出相同类型的重复用户
    pUser->KickOutSameClientType(pMsgConn->GetClientType(), ttidlbase::KICK_REASON_DUPLICATE_USER, pMsgConn);

    // 5-4.获取路由服务器连接
    // pRouteConn，如果存在则向路由服务器发送踢出用户的消息
    CRouteServConn* pRouteConn = get_route_serv_conn();
    if (pRouteConn) {
      ttidlserver::IMServerKickUser msg2;
      msg2.set_user_id(user_id);
      msg2.set_client_type((ttidlbase::ClientType)pMsgConn->GetClientType());
      msg2.set_reason(1);
      ttnetlib::CImPdu pdu;
      pdu.SetPBMsg(&msg2);
      pdu.SetServiceId(ttidlbase::SID_OTHER);
      pdu.SetCommandId(ttidlbase::CID_OTHER_SERVER_KICK_USER);  //踢出用户的消息
      pRouteConn->SendPdu(&pdu);
    }
    log_info("user_name: %s, uid: %d", login_name.c_str(), user_id);

    // 5-5.设置连接的用户ID、打开状态，并发送用户状态更新的消息
    pMsgConn->SetUserId(user_id);
    pMsgConn->SetOpen();
    pMsgConn->SendUserStatusUpdate(ttidlbase::USER_STATUS_ONLINE);
    pUser->ValidateMsgConn(pMsgConn->GetHandle(), pMsgConn);

    // 5-6.构造登录响应消息 msg3，设置相关字段
    ttidllogin::IMLoginRes msg3;
    msg3.set_server_time(time(NULL));
    msg3.set_result_code(ttidlbase::REFUSE_REASON_NONE);
    msg3.set_result_string(result_string);
    msg3.set_online_status((ttidlbase::UserStatType)pMsgConn->GetOnlineStatus());
    ttidlbase::UserInfo* user_info_tmp = msg3.mutable_user_info();
    user_info_tmp->set_user_id(user_info.user_id());
    user_info_tmp->set_user_gender(user_info.user_gender());
    user_info_tmp->set_user_nick_name(user_info.user_nick_name());
    user_info_tmp->set_avatar_url(user_info.avatar_url());
    user_info_tmp->set_sign_info(user_info.sign_info());
    user_info_tmp->set_department_id(user_info.department_id());
    user_info_tmp->set_email(user_info.email());
    user_info_tmp->set_user_real_name(user_info.user_real_name());
    user_info_tmp->set_user_tel(user_info.user_tel());
    user_info_tmp->set_user_domain(user_info.user_domain());
    user_info_tmp->set_status(user_info.status());

    // 5-6.构造登录响应消息Pdu pdu2，设置相关字段
    ttnetlib::CImPdu pdu2;
    pdu2.SetPBMsg(&msg3);                        //消息体
    pdu2.SetServiceId(ttidlbase::SID_LOGIN);                // service_id
    pdu2.SetCommandId(ttidlbase::CID_LOGIN_RES_USERLOGIN);  //新的command_id ttidlbase::CID_LOGIN_RES_USERLOGIN
    pdu2.SetSeqNum(pPdu->GetSeqNum());           //设置消息序号

    // 5-7.发送登录响应消息给客户端
    pMsgConn->SendPdu(&pdu2);
  } else {
    // 6.如果结果码不为零，表示验证失败，继续处理验证失败的逻辑
    // 6-1.构造登录响应消息 msg4，设置相关字段
    ttidllogin::IMLoginRes msg4;
    msg4.set_server_time(time(NULL));
    msg4.set_result_code((ttidlbase::ResultType)result);
    msg4.set_result_string(result_string);

    // 6-2.构造登录响应消息Pdu pdu2，设置相关字段
    ttnetlib::CImPdu pdu3;
    pdu3.SetPBMsg(&msg4);                        //消息体
    pdu3.SetServiceId(ttidlbase::SID_LOGIN);                // service_id
    pdu3.SetCommandId(ttidlbase::CID_LOGIN_RES_USERLOGIN);  // command_id ttidlbase::CID_LOGIN_RES_USERLOGIN
    pdu3.SetSeqNum(pPdu->GetSeqNum());
    pMsgConn->SendPdu(&pdu3);

    // 6-3.关闭连接
    pMsgConn->Close();
  }
}

void CDBServConn::_HandleRecentSessionResponse(ttnetlib::CImPdu* pPdu) {
  ttidlbuddy::IMRecentContactSessionRsp msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
  uint32_t user_id = msg.user_id();
  uint32_t session_cnt = msg.contact_session_list_size();
  CDbAttachData attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
  uint32_t handle = attach_data.GetHandle();

  log_info("HandleRecentSessionResponse, userId=%u, session_cnt=%u", user_id, session_cnt);

  CMsgConn* pMsgConn = ttuser::CImUserManager::GetInstance()->GetMsgConnByHandle(user_id, handle);

  if (pMsgConn && pMsgConn->IsOpen()) {
    msg.clear_attach_data();
    pPdu->SetPBMsg(&msg);
    pMsgConn->SendPdu(pPdu);
  }
}

void CDBServConn::_HandleAllUserResponse(ttnetlib::CImPdu* pPdu) {
  ttidlbuddy::IMAllUserRsp msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  uint32_t user_id = msg.user_id();
  uint32_t latest_update_time = msg.latest_update_time();
  uint32_t user_cnt = msg.user_list_size();
  CDbAttachData attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
  uint32_t handle = attach_data.GetHandle();

  log_info(
    "HandleAllUserResponse, userId=%u, latest_update_time=%u, user_cnt=%u", user_id, latest_update_time, user_cnt);

  CMsgConn* pMsgConn = ttuser::CImUserManager::GetInstance()->GetMsgConnByHandle(user_id, handle);

  if (pMsgConn && pMsgConn->IsOpen()) {
    msg.clear_attach_data();
    pPdu->SetPBMsg(&msg);
    pMsgConn->SendPdu(pPdu);
  }
}

void CDBServConn::_HandleGetMsgListResponse(ttnetlib::CImPdu* pPdu) {
  ttidlmessage::IMGetMsgListRsp msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  uint32_t user_id = msg.user_id();
  uint32_t session_type = msg.session_type();
  uint32_t session_id = msg.session_id();
  uint32_t msg_cnt = msg.msg_list_size();
  uint32_t msg_id_begin = msg.msg_id_begin();
  CDbAttachData attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
  uint32_t handle = attach_data.GetHandle();

  log_info(
    "HandleGetMsgListResponse, userId=%u, session_type=%u, "
    "opposite_user_id=%u, msg_id_begin=%u, cnt=%u.",
    user_id,
    session_type,
    session_id,
    msg_id_begin,
    msg_cnt);

  CMsgConn* pMsgConn = ttuser::CImUserManager::GetInstance()->GetMsgConnByHandle(user_id, handle);
  if (pMsgConn && pMsgConn->IsOpen()) {
    msg.clear_attach_data();
    pPdu->SetPBMsg(&msg);
    pMsgConn->SendPdu(pPdu);
  }
}

void CDBServConn::_HandleGetMsgByIdResponse(ttnetlib::CImPdu* pPdu) {
  ttidlmessage::IMGetMsgByIdRsp msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  uint32_t user_id = msg.user_id();
  uint32_t session_type = msg.session_type();
  uint32_t session_id = msg.session_id();
  uint32_t msg_cnt = msg.msg_list_size();
  CDbAttachData attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
  uint32_t handle = attach_data.GetHandle();

  log_info(
    "HandleGetMsgByIdResponse, userId=%u, session_type=%u, "
    "opposite_user_id=%u, cnt=%u.",
    user_id,
    session_type,
    session_id,
    msg_cnt);

  CMsgConn* pMsgConn = ttuser::CImUserManager::GetInstance()->GetMsgConnByHandle(user_id, handle);
  if (pMsgConn && pMsgConn->IsOpen()) {
    msg.clear_attach_data();
    pPdu->SetPBMsg(&msg);
    pMsgConn->SendPdu(pPdu);
  }
}

void CDBServConn::_HandleMsgData(ttnetlib::CImPdu* pPdu) {
  ttidlmessage::IMMsgData msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
  if (CHECK_MSG_TYPE_GROUP(msg.msg_type())) {
    s_group_chat->HandleGroupMessage(pPdu);
    return;
  }

  uint32_t from_user_id = msg.from_user_id();
  uint32_t to_user_id = msg.to_session_id();
  uint32_t msg_id = msg.msg_id();
  if (msg_id == 0) {
    log_info("HandleMsgData, write db failed, %u->%u.", from_user_id, to_user_id);
    return;
  }

  uint8_t msg_type = msg.msg_type();
  CDbAttachData attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
  uint32_t handle = attach_data.GetHandle();

  log_info("HandleMsgData, from_user_id=%u, to_user_id=%u, msg_id=%u.", from_user_id, to_user_id, msg_id);

  CMsgConn* pMsgConn = ttuser::CImUserManager::GetInstance()->GetMsgConnByHandle(from_user_id, attach_data.GetHandle());
  if (pMsgConn) {
    ttidlmessage::IMMsgDataAck msg2;
    msg2.set_user_id(from_user_id);
    msg2.set_msg_id(msg_id);
    msg2.set_session_id(to_user_id);
    msg2.set_session_type(ttidlbase::SESSION_TYPE_SINGLE);
    ttnetlib::CImPdu pdu;
    pdu.SetPBMsg(&msg2);
    pdu.SetServiceId(ttidlbase::SID_MSG);
    pdu.SetCommandId(ttidlbase::CID_MSG_DATA_ACK);
    pdu.SetSeqNum(pPdu->GetSeqNum());
    pMsgConn->SendPdu(&pdu);
  }

  CRouteServConn* pRouteConn = get_route_serv_conn();
  if (pRouteConn) {
    pRouteConn->SendPdu(pPdu);
  }

  msg.clear_attach_data();
  pPdu->SetPBMsg(&msg);
  ttuser::CImUser* pFromImUser = ttuser::CImUserManager::GetInstance()->GetImUserById(from_user_id);
  ttuser::CImUser* pToImUser = ttuser::CImUserManager::GetInstance()->GetImUserById(to_user_id);
  pPdu->SetSeqNum(0);
  if (pFromImUser) {
    pFromImUser->BroadcastClientMsgData(pPdu, msg_id, pMsgConn, from_user_id);
  }

  if (pToImUser) {
    pToImUser->BroadcastClientMsgData(pPdu, msg_id, NULL, from_user_id);
  }

  ttidlserver::IMGetDeviceTokenReq msg3;
  msg3.add_user_id(to_user_id);
  msg3.set_attach_data(pPdu->GetBodyData(), pPdu->GetBodyLength());
  ttnetlib::CImPdu pdu2;
  pdu2.SetPBMsg(&msg3);
  pdu2.SetServiceId(ttidlbase::SID_OTHER);
  pdu2.SetCommandId(ttidlbase::CID_OTHER_GET_DEVICE_TOKEN_REQ);
  SendPdu(&pdu2);
}

void CDBServConn::_HandleGetLatestMsgIDRsp(ttnetlib::CImPdu* pPdu) {
  ttidlmessage::IMGetLatestMsgIdRsp msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  uint32_t user_id = msg.user_id();
  uint32_t session_id = msg.session_id();
  uint32_t session_type = msg.session_type();
  uint32_t latest_msg_id = msg.latest_msg_id();
  CDbAttachData attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
  uint32_t handle = attach_data.GetHandle();

  log_info(
    "HandleUnreadMsgCntResp, userId=%u, session_id=%u, session_type=%u, "
    "latest_msg_id=%u.",
    user_id,
    session_id,
    session_type,
    latest_msg_id);

  CMsgConn* pMsgConn = ttuser::CImUserManager::GetInstance()->GetMsgConnByHandle(user_id, handle);
  if (pMsgConn && pMsgConn->IsOpen()) {
    msg.clear_attach_data();
    pPdu->SetPBMsg(&msg);
    pMsgConn->SendPdu(pPdu);
  }
}

void CDBServConn::_HandleUnreadMsgCountResponse(ttnetlib::CImPdu* pPdu) {
  ttidlmessage::IMUnreadMsgCntRsp msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  uint32_t user_id = msg.user_id();
  uint32_t total_cnt = msg.total_cnt();
  uint32_t user_unread_cnt = msg.unreadinfo_list_size();
  CDbAttachData attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
  uint32_t handle = attach_data.GetHandle();

  log_info("HandleUnreadMsgCntResp, userId=%u, total_cnt=%u, user_unread_cnt=%u.", user_id, total_cnt, user_unread_cnt);

  CMsgConn* pMsgConn = ttuser::CImUserManager::GetInstance()->GetMsgConnByHandle(user_id, handle);

  if (pMsgConn && pMsgConn->IsOpen()) {
    msg.clear_attach_data();
    pPdu->SetPBMsg(&msg);
    pMsgConn->SendPdu(pPdu);
  }
}

void CDBServConn::_HandleUsersInfoResponse(ttnetlib::CImPdu* pPdu) {
  ttidlbuddy::IMUsersInfoRsp msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  uint32_t user_id = msg.user_id();
  uint32_t user_cnt = msg.user_info_list_size();
  CDbAttachData attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
  uint32_t handle = attach_data.GetHandle();

  log_info("HandleUsersInfoResp, user_id=%u, user_cnt=%u.", user_id, user_cnt);

  CMsgConn* pMsgConn = ttuser::CImUserManager::GetInstance()->GetMsgConnByHandle(user_id, handle);
  if (pMsgConn && pMsgConn->IsOpen()) {
    msg.clear_attach_data();
    pPdu->SetPBMsg(&msg);
    pMsgConn->SendPdu(pPdu);
  }
}

void CDBServConn::_HandleStopReceivePacket(ttnetlib::CImPdu* pPdu) {
  log_info("HandleStopReceivePacket, from %s:%d.",
           g_db_server_list[m_serv_idx].server_ip.c_str(),
           g_db_server_list[m_serv_idx].server_port);

  m_bOpen = false;
}

void CDBServConn::_HandleRemoveSessionResponse(ttnetlib::CImPdu* pPdu) {
  ttidlbuddy::IMRemoveSessionRsp msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  uint32_t user_id = msg.user_id();
  uint32_t result = msg.result_code();
  uint32_t session_type = msg.session_type();
  uint32_t session_id = msg.session_id();
  log_info("HandleRemoveSessionResp, req_id=%u, result=%u, session_id=%u, type=%u.",
           user_id,
           result,
           session_id,
           session_type);

  CDbAttachData attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
  uint32_t handle = attach_data.GetHandle();
  CMsgConn* pConn = ttuser::CImUserManager::GetInstance()->GetMsgConnByHandle(user_id, handle);
  if (pConn && pConn->IsOpen()) {
    msg.clear_attach_data();
    pPdu->SetPBMsg(&msg);
    pConn->SendPdu(pPdu);
  }
}

void CDBServConn::_HandleChangeAvatarResponse(ttnetlib::CImPdu* pPdu) {
  ttidlbuddy::IMChangeAvatarRsp msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  uint32_t user_id = msg.user_id();
  uint32_t result = msg.result_code();

  log_info("HandleChangeAvatarResp, user_id=%u, result=%u.", user_id, result);

  ttuser::CImUser* pUser = ttuser::CImUserManager::GetInstance()->GetImUserById(user_id);
  if (NULL != pUser) {
    msg.clear_attach_data();
    pPdu->SetPBMsg(&msg);
    pUser->BroadcastPdu(pPdu);
  }
}

//处理部门信息的响应
void CDBServConn::_HandleDepartmentResponse(ttnetlib::CImPdu* pPdu) {
  // 1.解析收到的部门信息响应消息，将消息内容存储在 ttidlbuddy::IMDepartmentRsp
  // 类型的 msg 对象中
  ttidlbuddy::IMDepartmentRsp msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  // 2.获取用户ID、最新更新时间和部门数量等信息
  uint32_t user_id = msg.user_id();
  uint32_t latest_update_time = msg.latest_update_time();
  uint32_t dept_cnt = msg.dept_list_size();
  log_info(
    "HandleDepartmentResponse, user_id=%u, latest_update_time=%u, "
    "dept_cnt=%u.",
    user_id,
    latest_update_time,
    dept_cnt);

  // 3.解析附加数据，将其转换为 CDbAttachData 对象，获取句柄（handle）
  CDbAttachData attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());

  // 4.通过用户ID和句柄获取与之关联的消息连接对象 pConn
  uint32_t handle = attach_data.GetHandle();
  CMsgConn* pConn = ttuser::CImUserManager::GetInstance()->GetMsgConnByHandle(user_id, handle);

  // 5.如果消息连接对象存在且处于打开状态
  if (pConn && pConn->IsOpen()) {
    //清除附加数据中的内容
    msg.clear_attach_data();
    //设置响应消息的内容为解析后的 msg 对象
    pPdu->SetPBMsg(&msg);
    //将响应消息发送给消息连接对象
    pConn->SendPdu(pPdu);
  }
}

void CDBServConn::_HandleSetDeviceTokenResponse(ttnetlib::CImPdu* pPdu) {
  ttidllogin::IMDeviceTokenRsp msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  uint32_t user_id = msg.user_id();
  log_info("HandleSetDeviceTokenResponse, user_id = %u.", user_id);
}

void CDBServConn::_HandleGetDeviceTokenResponse(ttnetlib::CImPdu* pPdu) {
  ttidlserver::IMGetDeviceTokenRsp msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  ttidlmessage::IMMsgData msg2;
  CHECK_PB_PARSE_MSG(msg2.ParseFromArray(msg.attach_data().c_str(), msg.attach_data().length()));
  std::string msg_data = msg2.msg_data();
  uint32_t msg_type = msg2.msg_type();
  uint32_t from_id = msg2.from_user_id();
  uint32_t to_id = msg2.to_session_id();
  if (msg_type == ttidlbase::MSG_TYPE_SINGLE_TEXT || msg_type == ttidlbase::MSG_TYPE_GROUP_TEXT) {
    // msg_data =
    char* msg_out = NULL;
    uint32_t msg_out_len = 0;
    if (ttsecurity::DecryptMsg(msg_data.c_str(), msg_data.length(), &msg_out, msg_out_len) == 0) {
      msg_data = string(msg_out, msg_out_len);
    } else {
      log_info(
        "HandleGetDeviceTokenResponse, decrypt msg failed, from_id: %u, "
        "to_id: %u, msg_type: %u.",
        from_id,
        to_id,
        msg_type);
      return;
    }
    ttsecurity::Free(msg_out);
  }

  build_ios_push_flash(msg_data, msg2.msg_type(), from_id);
  //{
  //    "msg_type": 1,
  //    "from_id": "1345232",
  //    "group_type": "12353",
  //}
  Json::Value json_obj(Json::objectValue);
  json_obj["msg_type"] = static_cast<uint32_t>(msg2.msg_type());
  json_obj["from_id"] = from_id;
  if (CHECK_MSG_TYPE_GROUP(msg2.msg_type())) {
    json_obj["group_id"] = to_id;
  }

  Json::StreamWriterBuilder builder;
  builder["indentation"] = "";
  const std::string json_data = Json::writeString(builder, json_obj);

  uint32_t user_token_cnt = msg.user_token_info_size();
  log_info("HandleGetDeviceTokenResponse, user_token_cnt = %u.", user_token_cnt);

  ttidlserver::IMPushToUserReq msg3;
  for (uint32_t i = 0; i < user_token_cnt; i++) {
    ttidlbase::UserTokenInfo user_token = msg.user_token_info(i);
    uint32_t user_id = user_token.user_id();
    string device_token = user_token.token();
    uint32_t push_cnt = user_token.push_count();
    uint32_t client_type = user_token.user_type();
    // 自己发得消息不给自己发推送
    if (from_id == user_id) {
      continue;
    }

    log_info(
      "HandleGetDeviceTokenResponse, user_id = %u, device_token = %s, "
      "push_cnt = %u, client_type = %u.",
      user_id,
      device_token.c_str(),
      push_cnt,
      client_type);

    ttuser::CImUser* pUser = ttuser::CImUserManager::GetInstance()->GetImUserById(user_id);
    if (pUser) {
      msg3.set_flash(msg_data);
      msg3.set_data(json_data);
      ttidlbase::UserTokenInfo* user_token_tmp = msg3.add_user_token_list();
      user_token_tmp->set_user_id(user_id);
      user_token_tmp->set_user_type((ttidlbase::ClientType)client_type);
      user_token_tmp->set_token(device_token);
      user_token_tmp->set_push_count(push_cnt);
      // pc client登录，则为勿打扰式推送
      if (pUser->GetPCLoginStatus() == IM_PC_LOGIN_STATUS_ON) {
        user_token_tmp->set_push_type(IM_PUSH_TYPE_SILENT);
        log_info("HandleGetDeviceTokenResponse, user id: %d, push type: silent.", user_id);
      } else {
        user_token_tmp->set_push_type(IM_PUSH_TYPE_NORMAL);
        log_info("HandleGetDeviceTokenResponse, user id: %d, push type: normal.", user_id);
      }
    } else {
      ttidlserver::IMPushToUserReq msg4;
      msg4.set_flash(msg_data);
      msg4.set_data(json_data);
      ttidlbase::UserTokenInfo* user_token_tmp = msg4.add_user_token_list();
      user_token_tmp->set_user_id(user_id);
      user_token_tmp->set_user_type((ttidlbase::ClientType)client_type);
      user_token_tmp->set_token(device_token);
      user_token_tmp->set_push_count(push_cnt);
      user_token_tmp->set_push_type(IM_PUSH_TYPE_NORMAL);
      ttnetlib::CImPdu pdu;
      pdu.SetPBMsg(&msg4);
      pdu.SetServiceId(ttidlbase::SID_OTHER);
      pdu.SetCommandId(ttidlbase::CID_OTHER_PUSH_TO_USER_REQ);

      CPduAttachData attach_data(ATTACH_TYPE_PDU_FOR_PUSH, 0, pdu.GetBodyLength(), pdu.GetBodyData());
      ttidlbuddy::IMUsersStatReq msg5;
      msg5.set_user_id(0);
      msg5.add_user_id_list(user_id);
      msg5.set_attach_data(attach_data.GetBuffer(), attach_data.GetLength());
      ttnetlib::CImPdu pdu2;
      pdu2.SetPBMsg(&msg5);
      pdu2.SetServiceId(ttidlbase::SID_BUDDY_LIST);
      pdu2.SetCommandId(ttidlbase::CID_BUDDY_LIST_USERS_STATUS_REQUEST);
      CRouteServConn* route_conn = get_route_serv_conn();
      if (route_conn) {
        route_conn->SendPdu(&pdu2);
      }
    }
  }

  if (msg3.user_token_list_size() > 0) {
    ttnetlib::CImPdu pdu3;
    pdu3.SetPBMsg(&msg3);
    pdu3.SetServiceId(ttidlbase::SID_OTHER);
    pdu3.SetCommandId(ttidlbase::CID_OTHER_PUSH_TO_USER_REQ);

    CPushServConn* PushConn = get_push_serv_conn();
    if (PushConn) {
      PushConn->SendPdu(&pdu3);
    }
  }
}

void CDBServConn::_HandleChangeSignInfoResponse(ttnetlib::CImPdu* pPdu) {
  ttidlbuddy::IMChangeSignInfoRsp msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  uint32_t user_id = msg.user_id();
  uint32_t result = msg.result_code();

  log_info("HandleChangeSignInfoResp: user_id=%u, result=%u.", user_id, result);

  CDbAttachData attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
  uint32_t handle = attach_data.GetHandle();

  CMsgConn* pMsgConn = ttuser::CImUserManager::GetInstance()->GetMsgConnByHandle(user_id, handle);

  if (pMsgConn && pMsgConn->IsOpen()) {
    msg.clear_attach_data();
    pPdu->SetPBMsg(&msg);
    pMsgConn->SendPdu(pPdu);
  } else {
    log_info(
      "HandleChangeSignInfoResp: can't found msg_conn by user_id = %u, "
      "handle = %u",
      user_id,
      handle);
  }

  if (!result) {
    CRouteServConn* route_conn = get_route_serv_conn();
    if (route_conn) {
      ttidlbuddy::IMSignInfoChangedNotify notify_msg;
      notify_msg.set_changed_user_id(user_id);
      notify_msg.set_sign_info(msg.sign_info());

      ttnetlib::CImPdu notify_pdu;
      notify_pdu.SetPBMsg(&notify_msg);
      notify_pdu.SetServiceId(ttidlbase::SID_BUDDY_LIST);
      notify_pdu.SetCommandId(ttidlbase::CID_BUDDY_LIST_SIGN_INFO_CHANGED_NOTIFY);

      route_conn->SendPdu(&notify_pdu);
    } else {
      log_info("HandleChangeSignInfoResp: can't found route_conn");
    }
  }
}

void CDBServConn::_HandlePushShieldResponse(ttnetlib::CImPdu* pPdu) {
  ttidllogin::IMPushShieldRsp msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  uint32_t user_id = msg.user_id();
  uint32_t result = msg.result_code();

  log_info("_HandlePushShieldResponse: user_id=%u, result=%u.", user_id, result);

  CDbAttachData attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
  uint32_t handle = attach_data.GetHandle();

  CMsgConn* pMsgConn = ttuser::CImUserManager::GetInstance()->GetMsgConnByHandle(user_id, handle);

  if (pMsgConn && pMsgConn->IsOpen()) {
    msg.clear_attach_data();
    pPdu->SetPBMsg(&msg);
    pMsgConn->SendPdu(pPdu);
  } else {
    log_info(
      "_HandlePushShieldResponse: can't found msg_conn by user_id = %u, "
      "handle = %u",
      user_id,
      handle);
  }
}

void CDBServConn::_HandleQueryPushShieldResponse(ttnetlib::CImPdu* pPdu) {
  ttidllogin::IMQueryPushShieldRsp msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  uint32_t user_id = msg.user_id();
  uint32_t result = msg.result_code();
  // uint32_t shield_status = msg.shield_status();

  log_info("_HandleQueryPushShieldResponse: user_id=%u, result=%u.", user_id, result);

  CDbAttachData attach_data((uchar_t*)msg.attach_data().c_str(), msg.attach_data().length());
  uint32_t handle = attach_data.GetHandle();

  CMsgConn* pMsgConn = ttuser::CImUserManager::GetInstance()->GetMsgConnByHandle(user_id, handle);

  if (pMsgConn && pMsgConn->IsOpen()) {
    msg.clear_attach_data();
    pPdu->SetPBMsg(&msg);
    pMsgConn->SendPdu(pPdu);
  } else {
    log_info(
      "_HandleQueryPushShieldResponse: can't found msg_conn by user_id = %u, "
      "handle = %u",
      user_id,
      handle);
  }
}

}  // namespace teamtalk::msg_server::connection