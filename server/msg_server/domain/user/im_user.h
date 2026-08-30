/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ImUser.h
 Update Time: Thu 15 Jun 2023 00:55:30 CST
 brief:
*/

#ifndef TEAMTALK_MSG_SERVER_DOMAIN_USER_IM_USER_H_
#define TEAMTALK_MSG_SERVER_DOMAIN_USER_IM_USER_H_

#include <teamtalk/sbase/global_define.h>
#include <teamtalk/imcore/netlib/core/im_conn.h>

#include "connection/msg_conn.h"

// 最大在线好友数量
#define MAX_ONLINE_FRIEND_CNT 100

namespace teamtalk::msg_server::domain::user {

namespace ttconnection = teamtalk::msg_server::connection;
namespace ttnetlib = teamtalk::imcore::netlib;

class CImUser {
 public:
  CImUser(std::string user_name);
  ~CImUser();

  void SetUserId(uint32_t user_id) { m_user_id = user_id; }
  uint32_t GetUserId() { return m_user_id; }
  std::string GetLoginName() { return m_login_name; }
  void SetNickName(std::string nick_name) { m_nick_name = nick_name; }
  std::string GetNickName() { return m_nick_name; }
  bool IsValidate() { return m_bValidate; }
  void SetValidated() { m_bValidate = true; }
  uint32_t GetPCLoginStatus() { return m_pc_login_status; }
  void SetPCLoginStatus(uint32_t pc_login_status) { m_pc_login_status = pc_login_status; }

  user_conn_t GetUserConn();

  bool IsMsgConnEmpty() { return m_conn_map.empty(); }
  void AddMsgConn(uint32_t handle, ttconnection::CMsgConn* pMsgConn) { m_conn_map[handle] = pMsgConn; }
  void DelMsgConn(uint32_t handle) { m_conn_map.erase(handle); }
  ttconnection::CMsgConn* GetMsgConn(uint32_t handle);
  void ValidateMsgConn(uint32_t handle, ttconnection::CMsgConn* pMsgConn);

  void AddUnValidateMsgConn(ttconnection::CMsgConn* pMsgConn) { m_unvalidate_conn_set.insert(pMsgConn); }
  void DelUnValidateMsgConn(ttconnection::CMsgConn* pMsgConn) { m_unvalidate_conn_set.erase(pMsgConn); }
  ttconnection::CMsgConn* GetUnValidateMsgConn(uint32_t handle);

  std::map<uint32_t, ttconnection::CMsgConn*>& GetMsgConnMap() { return m_conn_map; }

  void BroadcastPdu(ttnetlib::CImPdu* pPdu, ttconnection::CMsgConn* pFromConn = nullptr);
  void BroadcastPduWithOutMobile(ttnetlib::CImPdu* pPdu, ttconnection::CMsgConn* pFromConn = nullptr);
  void BroadcastPduToMobile(ttnetlib::CImPdu* pPdu, ttconnection::CMsgConn* pFromConn = nullptr);
  void BroadcastClientMsgData(ttnetlib::CImPdu* pPdu,
                              uint32_t msg_id,
                              ttconnection::CMsgConn* pFromConn = nullptr,
                              uint32_t from_id = 0);
  void BroadcastData(void* buff, uint32_t len, ttconnection::CMsgConn* pFromConn = nullptr);

  void HandleKickUser(ttconnection::CMsgConn* pConn, uint32_t reason);

  bool KickOutSameClientType(uint32_t client_type, uint32_t reason, ttconnection::CMsgConn* pFromConn = nullptr);

  uint32_t GetClientTypeFlag();

 private:
  uint32_t m_user_id;
  std::string m_login_name; /* 登录名 */
  std::string m_nick_name;  /* 花名 */
  bool m_user_updated;
  uint32_t m_pc_login_status;  // pc client login状态，1: on 0: off

  bool m_bValidate;

  std::map<uint32_t, ttconnection::CMsgConn*> m_conn_map;
  std::set<ttconnection::CMsgConn*> m_unvalidate_conn_set;
};

typedef std::unordered_map<uint32_t /* user_id */, CImUser*> ImUserMap_t;
typedef std::unordered_map<std::string /* user_name */, CImUser*> ImUserMapByName_t;

}  // namespace teamtalk::msg_server::domain::user

#endif  // TEAMTALK_MSG_SERVER_DOMAIN_USER_IM_USER_H_