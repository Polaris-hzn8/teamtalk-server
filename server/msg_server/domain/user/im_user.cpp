/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ImUser.cpp
 Update Time: Thu 15 Jun 2023 00:55:22 CST
 brief:
*/

#include <teamtalk/imcore/ttidl/login.pb.h>
#include <teamtalk/imcore/slog/slog.h>

#include "domain/user/im_user.h"
#include "connection/msg_conn.h"
#include "connection/route_serv_conn.h"

using namespace std;

namespace teamtalk::msg_server::domain::user {

namespace ttconnection = teamtalk::msg_server::connection;
namespace ttidlbase = teamtalk::imcore::ttidl::base_define;
namespace ttidllogin = teamtalk::imcore::ttidl::login;

CImUser::CImUser(string user_name) {
  // log_info("ImUser, userId=%u\n", user_id);
  m_login_name = user_name;
  m_bValidate = false;
  m_user_id = 0;
  m_user_updated = false;
  m_pc_login_status = ttidlbase::USER_STATUS_OFFLINE;
}

CImUser::~CImUser() {
  // log_info("~ImUser, userId=%u\n", m_user_id);
}

ttconnection::CMsgConn* CImUser::GetUnValidateMsgConn(uint32_t handle) {
  for (set<ttconnection::CMsgConn*>::iterator it = m_unvalidate_conn_set.begin(); it != m_unvalidate_conn_set.end();
       it++) {
    ttconnection::CMsgConn* pConn = *it;
    if (pConn->GetHandle() == handle) {
      return pConn;
    }
  }
  return nullptr;
}

ttconnection::CMsgConn* CImUser::GetMsgConn(uint32_t handle) {
  ttconnection::CMsgConn* pMsgConn = nullptr;
  map<uint32_t, ttconnection::CMsgConn*>::iterator it = m_conn_map.find(handle);
  if (it != m_conn_map.end()) {
    pMsgConn = it->second;
  }
  return pMsgConn;
}

void CImUser::ValidateMsgConn(uint32_t handle, ttconnection::CMsgConn* pMsgConn) {
  AddMsgConn(handle, pMsgConn);
  DelUnValidateMsgConn(pMsgConn);
}

user_conn_t CImUser::GetUserConn() {
  uint32_t conn_cnt = 0;
  for (map<uint32_t, ttconnection::CMsgConn*>::iterator it = m_conn_map.begin(); it != m_conn_map.end(); it++) {
    ttconnection::CMsgConn* pConn = it->second;
    if (pConn->IsOpen()) {
      conn_cnt++;
    }
  }

  user_conn_t user_cnt = {m_user_id, conn_cnt};
  return user_cnt;
}

void CImUser::BroadcastPdu(ttnetlib::CImPdu* pPdu, ttconnection::CMsgConn* pFromConn) {
  for (map<uint32_t, ttconnection::CMsgConn*>::iterator it = m_conn_map.begin(); it != m_conn_map.end(); it++) {
    ttconnection::CMsgConn* pConn = it->second;
    if (pConn != pFromConn) {
      pConn->SendPdu(pPdu);
    }
  }
}

void CImUser::BroadcastPduWithOutMobile(ttnetlib::CImPdu* pPdu, ttconnection::CMsgConn* pFromConn) {
  for (map<uint32_t, ttconnection::CMsgConn*>::iterator it = m_conn_map.begin(); it != m_conn_map.end(); it++) {
    ttconnection::CMsgConn* pConn = it->second;
    if (pConn != pFromConn && CHECK_CLIENT_TYPE_PC(pConn->GetClientType())) {
      pConn->SendPdu(pPdu);
    }
  }
}

void CImUser::BroadcastPduToMobile(ttnetlib::CImPdu* pPdu, ttconnection::CMsgConn* pFromConn) {
  for (map<uint32_t, ttconnection::CMsgConn*>::iterator it = m_conn_map.begin(); it != m_conn_map.end(); it++) {
    ttconnection::CMsgConn* pConn = it->second;
    if (pConn != pFromConn && CHECK_CLIENT_TYPE_MOBILE(pConn->GetClientType())) {
      pConn->SendPdu(pPdu);
    }
  }
}

void CImUser::BroadcastClientMsgData(ttnetlib::CImPdu* pPdu,
                                     uint32_t msg_id,
                                     ttconnection::CMsgConn* pFromConn,
                                     uint32_t from_id) {
  for (map<uint32_t, ttconnection::CMsgConn*>::iterator it = m_conn_map.begin(); it != m_conn_map.end(); it++) {
    ttconnection::CMsgConn* pConn = it->second;
    if (pConn != pFromConn) {
      pConn->SendPdu(pPdu);
      pConn->AddToSendList(msg_id, from_id);
    }
  }
}

void CImUser::BroadcastData(void* buff, uint32_t len, ttconnection::CMsgConn* pFromConn) {
  if (!buff)
    return;
  for (map<uint32_t, ttconnection::CMsgConn*>::iterator it = m_conn_map.begin(); it != m_conn_map.end(); it++) {
    ttconnection::CMsgConn* pConn = it->second;

    if (pConn == nullptr)
      continue;

    if (pConn != pFromConn) {
      pConn->Send(buff, len);
    }
  }
}

void CImUser::HandleKickUser(ttconnection::CMsgConn* pConn, uint32_t reason) {
  map<uint32_t, ttconnection::CMsgConn*>::iterator it = m_conn_map.find(pConn->GetHandle());
  if (it != m_conn_map.end()) {
    ttconnection::CMsgConn* pConn = it->second;
    if (pConn) {
      log_info("kick service user, user_id=%u.", m_user_id);
      ttidllogin::IMKickUser msg;
      msg.set_user_id(m_user_id);
      msg.set_kick_reason((ttidlbase::KickReasonType)reason);
      ttnetlib::CImPdu pdu;
      pdu.SetPBMsg(&msg);
      pdu.SetServiceId(ttidlbase::SID_LOGIN);
      pdu.SetCommandId(ttidlbase::CID_LOGIN_KICK_USER);
      pConn->SendPdu(&pdu);
      pConn->SetKickOff();
      // pConn->Close();
    }
  }
}

// 只支持一个WINDOWS/MAC客户端登陆,或者一个ios/android登录
bool CImUser::KickOutSameClientType(uint32_t client_type, uint32_t reason, ttconnection::CMsgConn* pFromConn) {
  for (map<uint32_t, ttconnection::CMsgConn*>::iterator it = m_conn_map.begin(); it != m_conn_map.end(); it++) {
    ttconnection::CMsgConn* pMsgConn = it->second;

    // 16进制位移计算
    if ((((pMsgConn->GetClientType() ^ client_type) >> 4) == 0) && (pMsgConn != pFromConn)) {
      HandleKickUser(pMsgConn, reason);
      break;
    }
  }
  return true;
}

uint32_t CImUser::GetClientTypeFlag() {
  uint32_t client_type_flag = 0x00;
  map<uint32_t, ttconnection::CMsgConn*>::iterator it = m_conn_map.begin();
  for (; it != m_conn_map.end(); it++) {
    ttconnection::CMsgConn* pConn = it->second;
    uint32_t client_type = pConn->GetClientType();
    if (CHECK_CLIENT_TYPE_PC(client_type)) {
      client_type_flag |= CLIENT_TYPE_FLAG_PC;
    } else if (CHECK_CLIENT_TYPE_MOBILE(client_type)) {
      client_type_flag |= CLIENT_TYPE_FLAG_MOBILE;
    }
  }
  return client_type_flag;
}

}  // namespace teamtalk::msg_server::domain::user