/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: MsgConn.cpp
 Update Time: Thu 15 Jun 2023 00:56:53 CST
 brief:
*/

#include <teamtalk/sbase/global_define.h>
#include <teamtalk/imcore/netlib/core/im_pdu.h>

#include <teamtalk/imcore/ttidl/ohter.pb.h>
#include <teamtalk/imcore/ttidl/service.pb.h>
#include <teamtalk/imcore/ttidl/buddy.pb.h>
#include <teamtalk/imcore/ttidl/login.pb.h>
#include <teamtalk/imcore/ttidl/message.pb.h>
#include <teamtalk/imcore/ttidl/switch_service.pb.h>

#include "connection/msg_conn.h"
#include "connection/attach_data.h"
#include "connection/db_serv_conn.h"
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
namespace ttidllogin = teamtalk::imcore::ttidl::login;
namespace ttidlother = teamtalk::imcore::ttidl::other;
namespace ttidlmessage = teamtalk::imcore::ttidl::message;
namespace ttidlbuddy = teamtalk::imcore::ttidl::buddy;
namespace ttidlswitchservice = teamtalk::imcore::ttidl::switch_service;

#define TIMEOUT_WATI_LOGIN_RESPONSE 15000   // 15 seconds
#define TIMEOUT_WAITING_MSG_DATA_ACK 15000  // 15 seconds
#define LOG_MSG_STAT_INTERVAL 300000        // log message miss status in every 5 minutes;
#define MAX_MSG_CNT_PER_SECOND 20           // user can not send more than 20 msg in one second
static ttnetlib::ConnMap_t g_msg_conn_map;
static ttnetlib::UserMap_t g_msg_conn_user_map;

static uint64_t g_last_stat_tick;          // 上次显示丢包率信息的时间
static uint32_t g_up_msg_total_cnt = 0;    // 上行消息包总数
static uint32_t g_up_msg_miss_cnt = 0;     // 上行消息包丢数
static uint32_t g_down_msg_total_cnt = 0;  // 下行消息包总数
static uint32_t g_down_msg_miss_cnt = 0;   // 下行消息丢包数

static bool g_log_msg_toggle = true;  // 是否把收到的MsgData写入Log的开关，通过kill -SIGUSR2 pid 打开/关闭

static ttfile_handler::CFileHandler* s_file_handler = NULL;
static ttchat_handler::CGroupChat* s_group_chat = NULL;

void msg_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam) {
  ttnetlib::ConnMap_t::iterator it_old;
  CMsgConn* pConn = NULL;
  uint64_t cur_time = get_tick_count();

  for (ttnetlib::ConnMap_t::iterator it = g_msg_conn_map.begin(); it != g_msg_conn_map.end();) {
    it_old = it;
    it++;

    pConn = (CMsgConn*)it_old->second;
    pConn->OnTimer(cur_time);
  }

  if (cur_time > g_last_stat_tick + LOG_MSG_STAT_INTERVAL) {
    g_last_stat_tick = cur_time;
    log_info(
      "up_msg_cnt=%u, up_msg_miss_cnt=%u, down_msg_cnt=%u, "
      "down_msg_miss_cnt=%u ",
      g_up_msg_total_cnt,
      g_up_msg_miss_cnt,
      g_down_msg_total_cnt,
      g_down_msg_miss_cnt);
  }
}

static void signal_handler_usr1(int sig_no) {
  if (sig_no == SIGUSR1) {
    log_info("receive SIGUSR1 ");
    g_up_msg_total_cnt = 0;
    g_up_msg_miss_cnt = 0;
    g_down_msg_total_cnt = 0;
    g_down_msg_miss_cnt = 0;
  }
}

static void signal_handler_usr2(int sig_no) {
  if (sig_no == SIGUSR2) {
    log_info("receive SIGUSR2 ");
    g_log_msg_toggle = !g_log_msg_toggle;
  }
}

static void signal_handler_hup(int sig_no) {
  if (sig_no == SIGHUP) {
    log_info("receive SIGHUP exit... ");
    exit(0);
  }
}

void init_msg_conn() {
  g_last_stat_tick = get_tick_count();
  signal(SIGUSR1, signal_handler_usr1);
  signal(SIGUSR2, signal_handler_usr2);
  signal(SIGHUP, signal_handler_hup);
  netlib_register_timer(msg_conn_timer_callback, NULL, 1000);
  s_file_handler = ttfile_handler::CFileHandler::getInstance();
  s_group_chat = ttchat_handler::CGroupChat::GetInstance();
}

////////////////////////////
CMsgConn::CMsgConn() {
  m_user_id = 0;
  m_bOpen = false;
  m_bKickOff = false;
  m_last_seq_no = 0;
  m_msg_cnt_per_sec = 0;
  m_send_msg_list.clear();
  m_online_status = ttidlbase::USER_STATUS_OFFLINE;
}

CMsgConn::~CMsgConn() {}

void CMsgConn::SendUserStatusUpdate(uint32_t user_status) {
  if (!m_bOpen) {
    return;
  }

  ttuser::CImUser* pImUser = ttuser::CImUserManager::GetInstance()->GetImUserById(GetUserId());
  if (!pImUser) {
    return;
  }

  // 只有上下线通知才通知LoginServer
  if (user_status == ttidlbase::USER_STATUS_ONLINE) {
    ttidlserver::IMUserCntUpdate msg;
    msg.set_user_action(USER_CNT_INC);
    msg.set_user_id(pImUser->GetUserId());
    ttnetlib::CImPdu pdu;
    pdu.SetPBMsg(&msg);
    pdu.SetServiceId(SID_OTHER);
    pdu.SetCommandId(CID_OTHER_USER_CNT_UPDATE);
    send_to_all_login_server(&pdu);

    ttidlserver::IMUserStatusUpdate msg2;
    msg2.set_user_status(ttidlbase::USER_STATUS_ONLINE);
    msg2.set_user_id(pImUser->GetUserId());
    msg2.set_client_type((ttidlbase::ClientType)m_client_type);
    ttnetlib::CImPdu pdu2;
    pdu2.SetPBMsg(&msg2);
    pdu2.SetServiceId(SID_OTHER);
    pdu2.SetCommandId(CID_OTHER_USER_STATUS_UPDATE);

    send_to_all_route_server(&pdu2);
  } else if (user_status == ttidlbase::USER_STATUS_OFFLINE) {
    ttidlserver::IMUserCntUpdate msg;
    msg.set_user_action(USER_CNT_DEC);
    msg.set_user_id(pImUser->GetUserId());
    ttnetlib::CImPdu pdu;
    pdu.SetPBMsg(&msg);
    pdu.SetServiceId(SID_OTHER);
    pdu.SetCommandId(CID_OTHER_USER_CNT_UPDATE);
    send_to_all_login_server(&pdu);

    ttidlserver::IMUserStatusUpdate msg2;
    msg2.set_user_status(ttidlbase::USER_STATUS_OFFLINE);
    msg2.set_user_id(pImUser->GetUserId());
    msg2.set_client_type((ttidlbase::ClientType)m_client_type);
    ttnetlib::CImPdu pdu2;
    pdu2.SetPBMsg(&msg2);
    pdu2.SetServiceId(SID_OTHER);
    pdu2.SetCommandId(CID_OTHER_USER_STATUS_UPDATE);
    send_to_all_route_server(&pdu2);
  }
}

void CMsgConn::Close(bool kick_user) {
  log_info("Close client, handle=%d, user_id=%u ", m_handle, GetUserId());
  if (m_handle != NETLIB_INVALID_HANDLE) {
    netlib_close(m_handle);
    g_msg_conn_map.erase(m_handle);
  }

  ttuser::CImUser* pImUser = ttuser::CImUserManager::GetInstance()->GetImUserById(GetUserId());
  if (pImUser) {
    pImUser->DelMsgConn(GetHandle());  // 删除自己如果可能连接成功了
    pImUser->DelUnValidateMsgConn(this);

    SendUserStatusUpdate(ttidlbase::USER_STATUS_OFFLINE);
    if (pImUser->IsMsgConnEmpty()) {
      ttuser::CImUserManager::GetInstance()->RemoveImUser(pImUser);
    }
  }

  pImUser = ttuser::CImUserManager::GetInstance()->GetImUserByLoginName(GetLoginName());
  if (pImUser) {
    pImUser->DelUnValidateMsgConn(this);
    if (pImUser->IsMsgConnEmpty()) {  // 没有端连接
      ttuser::CImUserManager::GetInstance()->RemoveImUser(pImUser);
    }
  }

  ReleaseRef();
}

void CMsgConn::OnConnect(net_handle_t handle) {
  m_handle = handle;
  m_login_time = get_tick_count();

  g_msg_conn_map.insert(make_pair(handle, this));

  netlib_option(handle, NETLIB_OPT_SET_CALLBACK, (void*)imconn_callback);
  netlib_option(handle, NETLIB_OPT_SET_CALLBACK_DATA, (void*)&g_msg_conn_map);
  netlib_option(handle, NETLIB_OPT_GET_REMOTE_IP, (void*)&m_peer_ip);
  netlib_option(handle, NETLIB_OPT_GET_REMOTE_PORT, (void*)&m_peer_port);
}

void CMsgConn::OnClose() {
  log_info("Warning: peer closed. ");
  Close();
}

void CMsgConn::OnTimer(uint64_t curr_tick) {
  m_msg_cnt_per_sec = 0;

  if (CHECK_CLIENT_TYPE_MOBILE(GetClientType())) {
    if (curr_tick > m_last_recv_tick + MOBILE_CLIENT_TIMEOUT) {
      log_info("mobile client timeout, handle=%d, uid=%u ", m_handle, GetUserId());
      Close();
      return;
    }
  } else {
    if (curr_tick > m_last_recv_tick + CLIENT_TIMEOUT) {
      log_info("client timeout, handle=%d, uid=%u ", m_handle, GetUserId());
      Close();
      return;
    }
  }

  if (!IsOpen()) {
    if (curr_tick > m_login_time + TIMEOUT_WATI_LOGIN_RESPONSE) {
      log_info("login timeout, handle=%d, uid=%u ", m_handle, GetUserId());
      Close();
      return;
    }
  }

  list<msg_ack_t>::iterator it_old;
  for (list<msg_ack_t>::iterator it = m_send_msg_list.begin(); it != m_send_msg_list.end();) {
    msg_ack_t msg = *it;
    it_old = it;
    it++;
    if (curr_tick >= msg.timestamp + TIMEOUT_WAITING_MSG_DATA_ACK) {
      log_info("!!!a msg missed, msg_id=%u, %u->%u ", msg.msg_id, msg.from_id, GetUserId());
      g_down_msg_miss_cnt++;
      m_send_msg_list.erase(it_old);
    } else {
      break;
    }
  }
}

//处理收到的PDU 根据PDU的命令ID，将其分派给相应的处理函数
void CMsgConn::HandlePdu(ttnetlib::CImPdu* pPdu) {
  // 1.检查pPdu的命令ID是否为CID_OTHER_HEARTBEAT（心跳命令）如果不是心跳命令，则打印相应的日志信息
  if (pPdu->GetCommandId() != CID_OTHER_HEARTBEAT)
    log_info("HandlePdu cmd:0x%04x\n",
             pPdu->GetCommandId());  // request authorization check
  // 2.检查pPdu的命令ID是否为CID_LOGIN_REQ_USERLOGIN（用户登录请求）并且当前连接是打开的并且被踢出的状态
  if (pPdu->GetCommandId() != CID_LOGIN_REQ_USERLOGIN && !IsOpen() && IsKickOff()) {
    //如果条件满足则打印相应的日志信息，并抛出一个CPduException异常，异常信息指示处理PDU时出错用户未登录
    log_info("HandlePdu, wrong msg. ");
    throw CPduException(
      pPdu->GetServiceId(), pPdu->GetCommandId(), ERROR_CODE_WRONG_SERVICE_ID, "HandlePdu error, user not login. ");
    return;
  }
  // 3.最后通过switch语句根据pPdu的命令ID执行相应的操作
  // 根据不同的命令ID，调用相应的私有方法来处理不同的请求，例如处理心跳、用户登录请求、用户登出请求等
  switch (pPdu->GetCommandId()) {
    case CID_OTHER_HEARTBEAT:
      _HandleHeartBeat(pPdu);
      break;
    case CID_LOGIN_REQ_USERLOGIN:
      _HandleLoginRequest(pPdu);
      break;
    case CID_LOGIN_REQ_LOGINOUT:
      _HandleLoginOutRequest(pPdu);
      break;
    case CID_LOGIN_REQ_DEVICETOKEN:
      _HandleClientDeviceToken(pPdu);
      break;
    case CID_LOGIN_REQ_KICKPCCLIENT:
      _HandleKickPCClient(pPdu);
      break;
    case CID_LOGIN_REQ_PUSH_SHIELD:
      _HandlePushShieldRequest(pPdu);
      break;

    case CID_LOGIN_REQ_QUERY_PUSH_SHIELD:
      _HandleQueryPushShieldRequest(pPdu);
      break;
    case CID_LOGIN_REQ_REGIST:
      _HandleRegistRequest(pPdu);
      break;
    case CID_MSG_DATA:
      _HandleClientMsgData(pPdu);
      break;
    case CID_MSG_DATA_ACK:
      _HandleClientMsgDataAck(pPdu);
      break;
    case CID_MSG_TIME_REQUEST:
      _HandleClientTimeRequest(pPdu);
      break;
    case CID_MSG_LIST_REQUEST:
      _HandleClientGetMsgListRequest(pPdu);
      break;
    case CID_MSG_GET_BY_MSG_ID_REQ:
      _HandleClientGetMsgByMsgIdRequest(pPdu);
      break;
    case CID_MSG_UNREAD_CNT_REQUEST:
      _HandleClientUnreadMsgCntRequest(pPdu);
      break;
    case CID_MSG_READ_ACK:
      _HandleClientMsgReadAck(pPdu);
      break;
    case CID_MSG_GET_LATEST_MSG_ID_REQ:
      _HandleClientGetLatestMsgIDReq(pPdu);
      break;
    case CID_SWITCH_P2P_CMD:
      _HandleClientP2PCmdMsg(pPdu);
      break;
    case CID_BUDDY_LIST_RECENT_CONTACT_SESSION_REQUEST:
      _HandleClientRecentContactSessionRequest(pPdu);
      break;
    case CID_BUDDY_LIST_USER_INFO_REQUEST:
      _HandleClientUserInfoRequest(pPdu);
      break;
    case CID_BUDDY_LIST_REMOVE_SESSION_REQ:
      _HandleClientRemoveSessionRequest(pPdu);
      break;
    case CID_BUDDY_LIST_ALL_USER_REQUEST:
      _HandleClientAllUserRequest(pPdu);
      break;
    case CID_BUDDY_LIST_CHANGE_AVATAR_REQUEST:
      _HandleChangeAvatarRequest(pPdu);
      break;
    case CID_BUDDY_LIST_CHANGE_SIGN_INFO_REQUEST:
      _HandleChangeSignInfoRequest(pPdu);
      break;

    case CID_BUDDY_LIST_USERS_STATUS_REQUEST:
      _HandleClientUsersStatusRequest(pPdu);
      break;
    case CID_BUDDY_LIST_DEPARTMENT_REQUEST:
      _HandleClientDepartmentRequest(pPdu);
      break;
    // for group process
    case CID_GROUP_NORMAL_LIST_REQUEST:
      s_group_chat->HandleClientGroupNormalRequest(pPdu, this);
      break;
    case CID_GROUP_INFO_REQUEST:
      s_group_chat->HandleClientGroupInfoRequest(pPdu, this);
      break;
    case CID_GROUP_CREATE_REQUEST:
      s_group_chat->HandleClientGroupCreateRequest(pPdu, this);
      break;
    case CID_GROUP_CHANGE_MEMBER_REQUEST:
      s_group_chat->HandleClientGroupChangeMemberRequest(pPdu, this);
      break;
    case CID_GROUP_SHIELD_GROUP_REQUEST:
      s_group_chat->HandleClientGroupShieldGroupRequest(pPdu, this);
      break;

    case CID_FILE_REQUEST:
      s_file_handler->HandleClientFileRequest(this, pPdu);
      break;
    case CID_FILE_HAS_OFFLINE_REQ:
      s_file_handler->HandleClientFileHasOfflineReq(this, pPdu);
      break;
    case CID_FILE_ADD_OFFLINE_REQ:
      s_file_handler->HandleClientFileAddOfflineReq(this, pPdu);
      break;
    case CID_FILE_DEL_OFFLINE_REQ:
      s_file_handler->HandleClientFileDelOfflineReq(this, pPdu);
      break;
    default:
      log_info("wrong msg, cmd id=%d, user id=%u. ", pPdu->GetCommandId(), GetUserId());
      break;
  }
}

void CMsgConn::_HandleHeartBeat(ttnetlib::CImPdu* pPdu) {
  // 响应
  SendPdu(pPdu);
}

//向db_proxy_server发送用户登录认证请求
void CMsgConn::_HandleLoginRequest(ttnetlib::CImPdu* pPdu) {
  // 1.检查是否已经存在登录名
  // 如果存在则打印相应的日志信息，并直接返回拒绝重复的登录请求
  if (m_login_name.length() != 0) {
    log_info("duplicate LoginRequest in the same conn ");
    return;
  }

  // 2.检查各个服务器连接的可用性 包括数据库服务器连接和路由服务器连接
  uint32_t result = 0;
  string result_string = "";
  // 获取数据库服务器连接对象 pDbConn
  CDBServConn* pDbConn = get_db_serv_conn_for_login();
  if (!pDbConn) {
    //没有数据库服务器连接 表示拒绝登录
    result = ttidlbase::REFUSE_REASON_NO_DB_SERVER;
    result_string = "服务端异常";
  } else if (!is_login_server_available()) {
    //登录服务器不可用
    result = ttidlbase::REFUSE_REASON_NO_LOGIN_SERVER;
    result_string = "服务端异常";
  } else if (!is_route_server_available()) {
    //路由服务器不可用
    result = ttidlbase::REFUSE_REASON_NO_ROUTE_SERVER;
    result_string = "服务端异常";
  }

  // 3.如果存在拒绝登录的情况（result不为0）则会发送登录响应消息给客户端，并关闭连接
  if (result) {
    // 3-1.创建一个ttidllogin::IMLoginRes对象msg 并设置该对象的字段值
    ttidllogin::IMLoginRes msg;
    // 3-2.设置相关响应消息
    msg.set_server_time(time(NULL));                          //设置为当前时间
    msg.set_result_code((ttidlbase::ResultType)result);  //设置为result的值，表示拒绝原因
    msg.set_result_string(result_string);                     //设置为result_string的值，表示结果说明
    // 3-3.创建一个ttnetlib::CImPdu对象pdu，并将msg对象设置为其消息体
    ttnetlib::CImPdu pdu;
    pdu.SetPBMsg(&msg);                         //将 msg 对象设置为其消息体
    pdu.SetServiceId(SID_LOGIN);                //设置pdu的服务ID为SID_LOGIN
    pdu.SetCommandId(CID_LOGIN_RES_USERLOGIN);  //命令ID为CID_LOGIN_RES_USERLOGIN
    pdu.SetSeqNum(pPdu->GetSeqNum());           //消息序列号为登录请求消息的序列号
    // 3-4.调用SendPdu方法将登录响应消息发送给客户端
    SendPdu(&pdu);
    // 3-5.最后调用Close方法关闭连接结束处理
    Close();  // 关闭 CMsgConn* pConn = new CMsgConn(); 什么时候释放资源
    return;
  }

  // 4.服务器连接都正常，则继续处理登录请求
  ttidllogin::IMLoginReq msg;
  // 4-1.解析登录请求消息的内容
  // 该方法将消息的二进制数据解析为 ttidllogin::IMLoginReq 对象 msg
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  // 4-2.设置消息中的信息到私有成员变量中
  m_login_name = msg.user_name();    //登录请求中的用户名
  string password = msg.password();  //登录请求中的密码
  //检查在线状态的取值范围是否有效 如果不在有效范围内
  //会打印相应的日志信息，并将在线状态设置为默认值
  uint32_t online_status = msg.online_status();
  if (online_status < ttidlbase::USER_STATUS_ONLINE || online_status > ttidlbase::USER_STATUS_LEAVE) {
    log_info("HandleLoginReq, online status wrong: %u ", online_status);
    online_status = ttidlbase::USER_STATUS_ONLINE;
  }
  m_client_version = msg.client_version();  //客户端版本
  m_client_type = msg.client_type();        //客户端类型
  m_online_status = online_status;          //在线状态
  log_info("HandleLoginReq, user_name=%s, status=%u, client_type=%u, client=%s, ",
           m_login_name.c_str(),
           online_status,
           m_client_type,
           m_client_version.c_str());

  // 5.用户重复登录验证
  // 通过调用
  // ttuser::CImUserManager::GetInstance()->GetImUserByLoginName(GetLoginName()) 方法
  // 5-1.根据登录名获取对应的 CImUser 对象 pImUser
  ttuser::CImUser* pImUser = ttuser::CImUserManager::GetInstance()->GetImUserByLoginName(GetLoginName());

  // 5-2.如果pImUser为空，表示该用户尚未存在创建一个新的CImUser对象，并将其添加到用户管理器中
  if (!pImUser) {
    pImUser = new ttuser::CImUser(GetLoginName());  // 新建一个用户
    ttuser::CImUserManager::GetInstance()->AddImUserByLoginName(GetLoginName(), pImUser);
  }

  // 6.将验证消息转发到 db_proxy_server 进行身份验证
  // 6-1.将当前连接（this）添加到用户的未验证连接列表中
  pImUser->AddUnValidateMsgConn(this);

  // 6-2.创建 CDbAttachData 对象
  // attach_data，该对象用于传递附加数据给数据库代理服务器
  // 将连接的句柄（m_handle）作为附加数据，用于后续验证的过程中标识连接
  CDbAttachData attach_data(ATTACH_TYPE_HANDLE, m_handle, 0);

  // 6-3.创建ttidlserver::IMValidateReq对象msg2消息体，并将用户名、密码和附加数据设置到该对象中
  ttidlserver::IMValidateReq msg2;
  msg2.set_user_name(msg.user_name());  //用户名
  msg2.set_password(password);          //密码
  msg2.set_attach_data(attach_data.GetBuffer(),
                       attach_data.GetLength());  //附加数据

  // 6-4.设置pdu的服务ID为SID_OTHER，命令ID为CID_OTHER_VALIDATE_REQ，序列号为原始登录请求的序列号
  ttnetlib::CImPdu pdu;
  pdu.SetPBMsg(&msg2);
  pdu.SetServiceId(SID_OTHER);
  pdu.SetCommandId(CID_OTHER_VALIDATE_REQ);  //发送新的请求command_id CID_OTHER_VALIDATE_REQ
                                             //!!!!!!!!!!!!!!!
  pdu.SetSeqNum(pPdu->GetSeqNum());

  // 6-5.通过数据库代理连接对象pDbConn调用SendPdu()方法，将验证请求 pdu
  // 发送给数据库代理服务器进行验证
  pDbConn->SendPdu(&pdu);
}

void CMsgConn::_HandleLoginOutRequest(ttnetlib::CImPdu* pPdu) {
  log_info("HandleLoginOutRequest, user_id=%d, client_type=%u. ", GetUserId(), GetClientType());
  CDBServConn* pDBConn = get_db_serv_conn();
  if (pDBConn) {
    ttidllogin::IMDeviceTokenReq msg;
    msg.set_user_id(GetUserId());
    msg.set_device_token("");
    ttnetlib::CImPdu pdu;
    pdu.SetPBMsg(&msg);
    pdu.SetServiceId(SID_LOGIN);
    pdu.SetCommandId(CID_LOGIN_REQ_DEVICETOKEN);
    pdu.SetSeqNum(pPdu->GetSeqNum());
    pDBConn->SendPdu(&pdu);
  }

  ttidllogin::IMLogoutRsp msg2;
  msg2.set_result_code(0);
  ttnetlib::CImPdu pdu2;
  pdu2.SetPBMsg(&msg2);
  pdu2.SetServiceId(SID_LOGIN);
  pdu2.SetCommandId(CID_LOGIN_RES_LOGINOUT);
  pdu2.SetSeqNum(pPdu->GetSeqNum());
  SendPdu(&pdu2);
  Close();
}

void CMsgConn::_HandleKickPCClient(ttnetlib::CImPdu* pPdu) {
  ttidllogin::IMKickPCClientReq msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
  uint32_t user_id = GetUserId();
  if (!CHECK_CLIENT_TYPE_MOBILE(GetClientType())) {
    log_info("HandleKickPCClient, user_id = %u, cmd must come from mobile client. ", user_id);
    return;
  }
  log_info("HandleKickPCClient, user_id = %u. ", user_id);

  ttuser::CImUser* pImUser = ttuser::CImUserManager::GetInstance()->GetImUserById(user_id);
  if (pImUser) {
    pImUser->KickOutSameClientType(CLIENT_TYPE_MAC, ttidlbase::KICK_REASON_MOBILE_KICK, this);
  }

  CRouteServConn* pRouteConn = get_route_serv_conn();
  if (pRouteConn) {
    ttidlserver::IMServerKickUser msg2;
    msg2.set_user_id(user_id);
    msg2.set_client_type(ttidlbase::CLIENT_TYPE_MAC);
    msg2.set_reason(ttidlbase::KICK_REASON_MOBILE_KICK);
    ttnetlib::CImPdu pdu;
    pdu.SetPBMsg(&msg2);
    pdu.SetServiceId(SID_OTHER);
    pdu.SetCommandId(CID_OTHER_SERVER_KICK_USER);
    pRouteConn->SendPdu(&pdu);
  }

  ttidllogin::IMKickPCClientRsp msg2;
  msg2.set_user_id(user_id);
  msg2.set_result_code(0);
  ttnetlib::CImPdu pdu;
  pdu.SetPBMsg(&msg2);
  pdu.SetServiceId(SID_LOGIN);
  pdu.SetCommandId(CID_LOGIN_RES_KICKPCCLIENT);
  pdu.SetSeqNum(pPdu->GetSeqNum());
  SendPdu(&pdu);
}

void CMsgConn::_HandleClientRecentContactSessionRequest(ttnetlib::CImPdu* pPdu) {
  CDBServConn* pConn = get_db_serv_conn_for_login();
  if (!pConn) {
    return;
  }

  ttidlbuddy::IMRecentContactSessionReq msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
  log_info(
    "HandleClientRecentContactSessionRequest, user_id=%u, "
    "latest_update_time=%u. ",
    GetUserId(),
    msg.latest_update_time());

  msg.set_user_id(GetUserId());
  // 请求最近联系会话列表
  CDbAttachData attach_data(ATTACH_TYPE_HANDLE, m_handle, 0);
  msg.set_attach_data(attach_data.GetBuffer(), attach_data.GetLength());
  pPdu->SetPBMsg(&msg);
  pConn->SendPdu(pPdu);
}

//处理客户端发送的消息数据
void CMsgConn::_HandleClientMsgData(ttnetlib::CImPdu* pPdu) {
  // 1.解析收到的消息数据，将消息内容存储在 ttidlmessage::IMMsgData 类型的 msg
  // 对象中
  ttidlmessage::IMMsgData msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  // 2.检查消息数据是否为空，如果为空则记录日志并丢弃该消息
  if (msg.msg_data().length() == 0) {
    log_info("discard an empty message, uid=%u ", GetUserId());
    return;
  }

  // 3.检查每秒发送的消息数量是否超过阈值，如果超过则记录日志并丢弃该消息
  if (m_msg_cnt_per_sec >= MAX_MSG_CNT_PER_SECOND) {
    log_info("!!!too much msg cnt in one second, uid=%u ", GetUserId());
    return;
  }

  // 4.检查消息的发送者和接收者是否相同，并且消息类型是否为单聊类型，如果满足条件则记录日志并丢弃该消息
  if (msg.from_user_id() == msg.to_session_id() && CHECK_MSG_TYPE_SINGLE(msg.msg_type())) {
    log_info("!!!from_user_id == to_user_id. ");
    return;
  }

  //增加每秒发送的消息数量计数
  m_msg_cnt_per_sec++;

  // 5.获取消息的接收者ID、消息ID、消息类型和消息内容等信息
  uint32_t to_session_id = msg.to_session_id();
  uint32_t msg_id = msg.msg_id();
  uint8_t msg_type = msg.msg_type();
  string msg_data = msg.msg_data();

  // 如果日志记录开关打开，则记录日志，输出发送者ID、接收者ID、消息类型和消息ID等信息
  if (g_log_msg_toggle)
    log_info("HandleClientMsgData, %d->%d, msg_type=%u, msg_id=%u. ", GetUserId(), to_session_id, msg_type, msg_id);

  // 6.获取当前时间作为消息的创建时间
  uint32_t cur_time = time(NULL);

  // 7.创建 CDbAttachData 对象，将其类型设置为
  // ATTACH_TYPE_HANDLE，并存储句柄（handle）信息
  // 设置消息的发送者ID、创建时间和附加数据等信息
  CDbAttachData attach_data(ATTACH_TYPE_HANDLE, m_handle, 0);
  msg.set_from_user_id(GetUserId());
  msg.set_create_time(cur_time);
  // 将消息数据设置为解析后的 msg 对象
  msg.set_attach_data(attach_data.GetBuffer(), attach_data.GetLength());

  // 8.设置msg为消息体
  pPdu->SetPBMsg(&msg);

  // send to DB storage server
  // 9.将消息发送给 db_proxy_server
  CDBServConn* pDbConn = get_db_serv_conn();
  if (pDbConn)
    pDbConn->SendPdu(pPdu);
}

void CMsgConn::_HandleClientMsgDataAck(ttnetlib::CImPdu* pPdu) {
  ttidlmessage::IMMsgDataAck msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  ttidlbase::SessionType session_type = msg.session_type();
  if (session_type == ttidlbase::SESSION_TYPE_SINGLE) {
    uint32_t msg_id = msg.msg_id();
    uint32_t session_id = msg.session_id();
    DelFromSendList(msg_id, session_id);
  }
}

void CMsgConn::_HandleClientTimeRequest(ttnetlib::CImPdu* pPdu) {
  ttidlmessage::IMClientTimeRsp msg;
  msg.set_server_time((uint32_t)time(NULL));
  ttnetlib::CImPdu pdu;
  pdu.SetPBMsg(&msg);
  pdu.SetServiceId(SID_MSG);
  pdu.SetCommandId(CID_MSG_TIME_RESPONSE);
  pdu.SetSeqNum(pPdu->GetSeqNum());
  SendPdu(&pdu);
}

void CMsgConn::_HandleClientGetMsgListRequest(ttnetlib::CImPdu* pPdu) {
  ttidlmessage::IMGetMsgListReq msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
  uint32_t session_id = msg.session_id();
  uint32_t msg_id_begin = msg.msg_id_begin();
  uint32_t msg_cnt = msg.msg_cnt();
  uint32_t session_type = msg.session_type();
  log_info(
    "HandleClientGetMsgListRequest, req_id=%u, session_type=%u, "
    "session_id=%u, msg_id_begin=%u, msg_cnt=%u. ",
    GetUserId(),
    session_type,
    session_id,
    msg_id_begin,
    msg_cnt);
  CDBServConn* pDBConn = get_db_serv_conn_for_login();
  if (pDBConn) {
    CDbAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0);
    msg.set_user_id(GetUserId());
    msg.set_attach_data(attach.GetBuffer(), attach.GetLength());
    pPdu->SetPBMsg(&msg);
    pDBConn->SendPdu(pPdu);
  }
}

void CMsgConn::_HandleClientGetMsgByMsgIdRequest(ttnetlib::CImPdu* pPdu) {
  ttidlmessage::IMGetMsgByIdReq msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
  uint32_t session_id = msg.session_id();
  uint32_t session_type = msg.session_type();
  uint32_t msg_cnt = msg.msg_id_list_size();
  log_info(
    "_HandleClientGetMsgByMsgIdRequest, req_id=%u, session_type=%u, "
    "session_id=%u, msg_cnt=%u.",
    GetUserId(),
    session_type,
    session_id,
    msg_cnt);
  CDBServConn* pDBConn = get_db_serv_conn_for_login();
  if (pDBConn) {
    CDbAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0);
    msg.set_user_id(GetUserId());
    msg.set_attach_data(attach.GetBuffer(), attach.GetLength());
    pPdu->SetPBMsg(&msg);
    pDBConn->SendPdu(pPdu);
  }
}

void CMsgConn::_HandleClientUnreadMsgCntRequest(ttnetlib::CImPdu* pPdu) {
  log_info("HandleClientUnreadMsgCntReq, from_id=%u ", GetUserId());
  ttidlmessage::IMUnreadMsgCntReq msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));

  CDBServConn* pDBConn = get_db_serv_conn_for_login();
  if (pDBConn) {
    CDbAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0);
    msg.set_user_id(GetUserId());
    msg.set_attach_data(attach.GetBuffer(), attach.GetLength());
    pPdu->SetPBMsg(&msg);
    pDBConn->SendPdu(pPdu);
  }
}

void CMsgConn::_HandleClientMsgReadAck(ttnetlib::CImPdu* pPdu) {
  ttidlmessage::IMMsgDataReadAck msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
  uint32_t session_type = msg.session_type();
  uint32_t session_id = msg.session_id();
  uint32_t msg_id = msg.msg_id();
  log_info(
    "HandleClientMsgReadAck, user_id=%u, session_id=%u, msg_id=%u, "
    "session_type=%u. ",
    GetUserId(),
    session_id,
    msg_id,
    session_type);

  CDBServConn* pDBConn = get_db_serv_conn();
  if (pDBConn) {
    msg.set_user_id(GetUserId());
    pPdu->SetPBMsg(&msg);
    pDBConn->SendPdu(pPdu);
  }
  ttidlmessage::IMMsgDataReadNotify msg2;
  msg2.set_user_id(GetUserId());
  msg2.set_session_id(session_id);
  msg2.set_msg_id(msg_id);
  msg2.set_session_type((ttidlbase::SessionType)session_type);
  ttnetlib::CImPdu pdu;
  pdu.SetPBMsg(&msg2);
  pdu.SetServiceId(SID_MSG);
  pdu.SetCommandId(CID_MSG_READ_NOTIFY);
  ttuser::CImUser* pUser = ttuser::CImUserManager::GetInstance()->GetImUserById(GetUserId());
  if (pUser) {
    pUser->BroadcastPdu(&pdu, this);
  }
  CRouteServConn* pRouteConn = get_route_serv_conn();
  if (pRouteConn) {
    pRouteConn->SendPdu(&pdu);
  }

  if (session_type == ttidlbase::SESSION_TYPE_SINGLE) {
    DelFromSendList(msg_id, session_id);
  }
}

void CMsgConn::_HandleClientGetLatestMsgIDReq(ttnetlib::CImPdu* pPdu) {
  ttidlmessage::IMGetLatestMsgIdReq msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
  uint32_t session_type = msg.session_type();
  uint32_t session_id = msg.session_id();
  log_info(
    "HandleClientGetMsgListRequest, user_id=%u, session_id=%u, "
    "session_type=%u. ",
    GetUserId(),
    session_id,
    session_type);

  CDBServConn* pDBConn = get_db_serv_conn();
  if (pDBConn) {
    CDbAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0);
    msg.set_user_id(GetUserId());
    msg.set_attach_data(attach.GetBuffer(), attach.GetLength());
    pPdu->SetPBMsg(&msg);
    pDBConn->SendPdu(pPdu);
  }
}

void CMsgConn::_HandleClientP2PCmdMsg(ttnetlib::CImPdu* pPdu) {
  ttidlswitchservice::IMP2PCmdMsg msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
  string cmd_msg = msg.cmd_msg_data();
  uint32_t from_user_id = msg.from_user_id();
  uint32_t to_user_id = msg.to_user_id();

  log_info("HandleClientP2PCmdMsg, %u->%u, cmd_msg: %s ", from_user_id, to_user_id, cmd_msg.c_str());

  ttuser::CImUser* pFromImUser = ttuser::CImUserManager::GetInstance()->GetImUserById(GetUserId());
  ttuser::CImUser* pToImUser = ttuser::CImUserManager::GetInstance()->GetImUserById(to_user_id);

  if (pFromImUser) {
    pFromImUser->BroadcastPdu(pPdu, this);
  }

  if (pToImUser) {
    pToImUser->BroadcastPdu(pPdu, NULL);
  }

  CRouteServConn* pRouteConn = get_route_serv_conn();
  if (pRouteConn) {
    pRouteConn->SendPdu(pPdu);
  }
}

void CMsgConn::_HandleClientUserInfoRequest(ttnetlib::CImPdu* pPdu) {
  ttidlbuddy::IMUsersInfoReq msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
  uint32_t user_cnt = msg.user_id_list_size();
  log_info("HandleClientUserInfoReq, req_id=%u, user_cnt=%u ", GetUserId(), user_cnt);
  CDBServConn* pDBConn = get_db_serv_conn_for_login();
  if (pDBConn) {
    CDbAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0);
    msg.set_user_id(GetUserId());
    msg.set_attach_data(attach.GetBuffer(), attach.GetLength());
    pPdu->SetPBMsg(&msg);
    pDBConn->SendPdu(pPdu);
  }
}

void CMsgConn::_HandleClientRemoveSessionRequest(ttnetlib::CImPdu* pPdu) {
  ttidlbuddy::IMRemoveSessionReq msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
  uint32_t session_type = msg.session_type();
  uint32_t session_id = msg.session_id();
  log_info("HandleClientRemoveSessionReq, user_id=%u, session_id=%u, type=%u ", GetUserId(), session_id, session_type);

  CDBServConn* pConn = get_db_serv_conn();
  if (pConn) {
    CDbAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0);
    msg.set_user_id(GetUserId());
    msg.set_attach_data(attach.GetBuffer(), attach.GetLength());
    pPdu->SetPBMsg(&msg);
    pConn->SendPdu(pPdu);
  }

  if (session_type == ttidlbase::SESSION_TYPE_SINGLE) {
    ttidlbuddy::IMRemoveSessionNotify msg2;
    msg2.set_user_id(GetUserId());
    msg2.set_session_id(session_id);
    msg2.set_session_type((ttidlbase::SessionType)session_type);
    ttnetlib::CImPdu pdu;
    pdu.SetPBMsg(&msg2);
    pdu.SetServiceId(SID_BUDDY_LIST);
    pdu.SetCommandId(CID_BUDDY_LIST_REMOVE_SESSION_NOTIFY);
    ttuser::CImUser* pImUser = ttuser::CImUserManager::GetInstance()->GetImUserById(GetUserId());
    if (pImUser) {
      pImUser->BroadcastPdu(&pdu, this);
    }
    CRouteServConn* pRouteConn = get_route_serv_conn();
    if (pRouteConn) {
      pRouteConn->SendPdu(&pdu);
    }
  }
}

void CMsgConn::_HandleClientAllUserRequest(ttnetlib::CImPdu* pPdu) {
  ttidlbuddy::IMAllUserReq msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
  uint32_t latest_update_time = msg.latest_update_time();
  log_info("HandleClientAllUserReq, user_id=%u, latest_update_time=%u. ", GetUserId(), latest_update_time);

  CDBServConn* pConn = get_db_serv_conn();
  if (pConn) {
    CDbAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0);
    msg.set_user_id(GetUserId());
    msg.set_attach_data(attach.GetBuffer(), attach.GetLength());
    pPdu->SetPBMsg(&msg);
    pConn->SendPdu(pPdu);
  }
}

void CMsgConn::_HandleChangeAvatarRequest(ttnetlib::CImPdu* pPdu) {
  ttidlbuddy::IMChangeAvatarReq msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
  log_info("HandleChangeAvatarRequest, user_id=%u ", GetUserId());
  CDBServConn* pDBConn = get_db_serv_conn();
  if (pDBConn) {
    msg.set_user_id(GetUserId());
    pPdu->SetPBMsg(&msg);
    pDBConn->SendPdu(pPdu);
  }
}

void CMsgConn::_HandleClientUsersStatusRequest(ttnetlib::CImPdu* pPdu) {
  ttidlbuddy::IMUsersStatReq msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
  uint32_t user_count = msg.user_id_list_size();
  log_info("HandleClientUsersStatusReq, user_id=%u, query_count=%u.", GetUserId(), user_count);

  CRouteServConn* pRouteConn = get_route_serv_conn();
  if (pRouteConn) {
    msg.set_user_id(GetUserId());
    CPduAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0, NULL);
    msg.set_attach_data(attach.GetBuffer(), attach.GetLength());
    pPdu->SetPBMsg(&msg);
    pRouteConn->SendPdu(pPdu);
  }
}

//处理客户端部门信息请求
//当客户端发送部门信息请求时，服务器会调用此函数进行处理
void CMsgConn::_HandleClientDepartmentRequest(ttnetlib::CImPdu* pPdu) {
  ttidlbuddy::IMDepartmentReq msg;
  // 1.解析部门信息请求消息，获取用户ID和最新更新时间
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
  log_info("HandleClientDepartmentRequest, user_id=%u, latest_update_time=%u.", GetUserId(), msg.latest_update_time());

  // 2.获取数据库服务器连接 pDBConn
  CDBServConn* pDBConn = get_db_serv_conn();

  // 3.如果存在数据库服务器连接 pDBConn，则进行以下操作
  if (pDBConn) {
    // 3-1.创建CDbAttachData对象attach，用于存储附加数据
    // 附加数据类型为 ATTACH_TYPE_HANDLE，句柄为当前连接的句柄，长度为0
    CDbAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0);
    // 3-2.设置部门信息请求消息的用户ID和附加数据
    msg.set_user_id(GetUserId());
    msg.set_attach_data(attach.GetBuffer(), attach.GetLength());
    // 3-3.将部门信息请求消息设置为待发送的消息对象 pPdu 的 消息体
    pPdu->SetPBMsg(&msg);
    // 3-4.调用数据库服务器连接 pDBConn的SendPdu方法将数据发送到 db_proxy_server
    pDBConn->SendPdu(pPdu);
  }
}

void CMsgConn::_HandleClientDeviceToken(ttnetlib::CImPdu* pPdu) {
  if (!CHECK_CLIENT_TYPE_MOBILE(GetClientType())) {
    log_info("HandleClientDeviceToken, user_id=%u, not mobile client.", GetUserId());
    return;
  }
  ttidllogin::IMDeviceTokenReq msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
  string device_token = msg.device_token();
  log_info("HandleClientDeviceToken, user_id=%u, device_token=%s ", GetUserId(), device_token.c_str());

  ttidllogin::IMDeviceTokenRsp msg2;
  msg.set_user_id(GetUserId());
  msg.set_client_type((ttidlbase::ClientType)GetClientType());
  ttnetlib::CImPdu pdu;
  pdu.SetPBMsg(&msg2);
  pdu.SetServiceId(SID_LOGIN);
  pdu.SetCommandId(CID_LOGIN_RES_DEVICETOKEN);
  pdu.SetSeqNum(pPdu->GetSeqNum());
  SendPdu(&pdu);

  CDBServConn* pDBConn = get_db_serv_conn();
  if (pDBConn) {
    msg.set_user_id(GetUserId());
    pPdu->SetPBMsg(&msg);
    pDBConn->SendPdu(pPdu);
  }
}

void CMsgConn::AddToSendList(uint32_t msg_id, uint32_t from_id) {
  // log_info("AddSendMsg, seq_no=%u, from_id=%u ", seq_no, from_id);
  msg_ack_t msg;
  msg.msg_id = msg_id;
  msg.from_id = from_id;
  msg.timestamp = get_tick_count();
  m_send_msg_list.push_back(msg);

  g_down_msg_total_cnt++;
}

void CMsgConn::DelFromSendList(uint32_t msg_id, uint32_t from_id) {
  // log_info("DelSendMsg, seq_no=%u, from_id=%u ", seq_no, from_id);
  for (list<msg_ack_t>::iterator it = m_send_msg_list.begin(); it != m_send_msg_list.end(); it++) {
    msg_ack_t msg = *it;
    if ((msg.msg_id == msg_id) && (msg.from_id == from_id)) {
      m_send_msg_list.erase(it);
      break;
    }
  }
}

uint32_t CMsgConn::GetClientTypeFlag() {
  uint32_t client_type_flag = 0x00;
  if (CHECK_CLIENT_TYPE_PC(GetClientType())) {
    client_type_flag = CLIENT_TYPE_FLAG_PC;
  } else if (CHECK_CLIENT_TYPE_MOBILE(GetClientType())) {
    client_type_flag = CLIENT_TYPE_FLAG_MOBILE;
  }
  return client_type_flag;
}

void CMsgConn::_HandleChangeSignInfoRequest(ttnetlib::CImPdu* pPdu) {
  ttidlbuddy::IMChangeSignInfoReq msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
  log_info("HandleChangeSignInfoRequest, user_id=%u ", GetUserId());
  CDBServConn* pDBConn = get_db_serv_conn();
  if (pDBConn) {
    msg.set_user_id(GetUserId());
    CPduAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0, NULL);
    msg.set_attach_data(attach.GetBuffer(), attach.GetLength());

    pPdu->SetPBMsg(&msg);
    pDBConn->SendPdu(pPdu);
  }
}
void CMsgConn::_HandlePushShieldRequest(ttnetlib::CImPdu* pPdu) {
  ttidllogin::IMPushShieldReq msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
  log_info("_HandlePushShieldRequest, user_id=%u, shield_status ", GetUserId(), msg.shield_status());
  CDBServConn* pDBConn = get_db_serv_conn();
  if (pDBConn) {
    msg.set_user_id(GetUserId());
    CPduAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0, NULL);
    msg.set_attach_data(attach.GetBuffer(), attach.GetLength());

    pPdu->SetPBMsg(&msg);
    pDBConn->SendPdu(pPdu);
  }
}

void CMsgConn::_HandleQueryPushShieldRequest(ttnetlib::CImPdu* pPdu) {
  ttidllogin::IMQueryPushShieldReq msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
  log_info("HandleChangeSignInfoRequest, user_id=%u ", GetUserId());
  CDBServConn* pDBConn = get_db_serv_conn();
  if (pDBConn) {
    msg.set_user_id(GetUserId());
    CPduAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0, NULL);
    msg.set_attach_data(attach.GetBuffer(), attach.GetLength());

    pPdu->SetPBMsg(&msg);
    pDBConn->SendPdu(pPdu);
  }
}
void CMsgConn::_HandleRegistRequest(ttnetlib::CImPdu* pPdu) {
  // refuse second regist request
  uint64_t cur_time = get_tick_count();
  /*
  if (m_regist_time > cur_time - 5000) {  // 5秒内不能重复注册
      log_warn("duplicate RegistRequest in the same conn in 5 seconds");
      return;
  }*/

  uint32_t result = 0;
  string result_string = "";
  ttidllogin::IMRegistReq msg;
  CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
  log_info(
    "user_name=%s, password=%s, sex=%u, nick=%s, avatar=%s, phone=%s, "
    "email=%s",
    msg.user_name().c_str(),
    msg.password().c_str(),
    msg.sex(),
    msg.nick().c_str(),
    msg.avatar().c_str(),
    msg.phone().c_str(),
    msg.email().c_str());

  /*
     CDBServConn* pDbConn = get_db_serv_conn_for_login();
     if (0 == result) {
         // check if db server connection is OK
         if (!pDbConn) {
             result = ttidlbase::RESULT_PARAM_ERROR;
             result_string = "server exception";
         }
     }

     if (result) {
         log_error("app_id=%u, user_id=%u, user_name=%s, result=%d,
     result_string=%s", msg.app_id(), msg.user_id(), msg.user_name().c_str(),
     result, result_string.c_str()); ttidllogin::IMRegistRes msg;
         msg.set_result_code((ttidlbase::ResultType)result);
         msg.set_result_string(result_string);
         ttnetlib::CImPdu pdu;
         pdu.SetPBMsg(&msg);
         pdu.SetFlag(m_app_id);
         pdu.SetServiceId(SID_LOGIN);
         pdu.SetCommandId(CID_LOGIN_RES_REGIST);
         pdu.SetSeqNum(pPdu->GetSeqNum());
         SendPdu(&pdu);
         Close();
         return;
     }

     ttuser::CImUser* pImUser =
     ttuser::CImUserManager::GetInstance()->GetImUserByLoginName(msg.app_id(),
     msg.user_name()); if (!pImUser) { pImUser = new ttuser::CImUser(msg.user_name());
         ttuser::CImUserManager::GetInstance()->AddImUserByLoginName(msg.app_id(),
     msg.user_name(), pImUser);
     }
     pImUser->AddUnValidateMsgConn(this);

     CDbAttachData attach_data(ATTACH_TYPE_HANDLE, m_handle, 0);
     msg.set_attach_data(attach_data.GetBuffer(), attach_data.GetLength());
     ttnetlib::CImPdu pdu;
     pdu.SetPBMsg(&msg);
     pdu.SetFlag(m_app_id);
     pdu.SetServiceId(SID_LOGIN);
     pdu.SetCommandId(CID_LOGIN_REQ_REGIST);
     pdu.SetSeqNum(pPdu->GetSeqNum());
     pDbConn->SendPdu(&pdu);
     */
}

}  // namespace teamtalk::msg_server::connection