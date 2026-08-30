/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: GroupChat.h
 Update Time: Thu 15 Jun 2023 00:55:13 CST
 brief:
*/

#ifndef TEAMTALK_MSG_SERVER_SERVICE_CHAT_HANDLER_GROUP_CHAT_H_
#define TEAMTALK_MSG_SERVER_SERVICE_CHAT_HANDLER_GROUP_CHAT_H_

#include <map>
#include <set>
#include <unordered_map>
#include <teamtalk/imcore/netlib/core/im_pdu.h>

#include "connection/msg_conn.h"

namespace teamtalk::msg_server::service::chat_handler {

namespace ttconnection = teamtalk::msg_server::connection;
namespace ttnetlib = teamtalk::imcore::netlib;

typedef std::set<uint32_t> group_member_t;
typedef std::unordered_map<uint32_t, group_member_t*> group_map_t;

class CGroupChat {
 public:
  virtual ~CGroupChat() {}

  static CGroupChat* GetInstance();

  void HandleClientGroupNormalRequest(ttnetlib::CImPdu* pPdu, ttconnection::CMsgConn* pFromConn);
  void HandleGroupNormalResponse(ttnetlib::CImPdu* pPdu);

  void HandleClientGroupInfoRequest(ttnetlib::CImPdu* pPdu, ttconnection::CMsgConn* pFromConn);
  void HandleGroupInfoResponse(ttnetlib::CImPdu* pPdu);

  void HandleGroupMessage(ttnetlib::CImPdu* pPdu);
  void HandleGroupMessageBroadcast(ttnetlib::CImPdu* pPdu);

  void HandleClientGroupCreateRequest(ttnetlib::CImPdu* pPdu, ttconnection::CMsgConn* pFromConn);
  void HandleGroupCreateResponse(ttnetlib::CImPdu* pPdu);

  void HandleClientGroupChangeMemberRequest(ttnetlib::CImPdu* pPdu, ttconnection::CMsgConn* pFromConn);
  void HandleGroupChangeMemberResponse(ttnetlib::CImPdu* pPdu);
  void HandleGroupChangeMemberBroadcast(ttnetlib::CImPdu* pPdu);

  void HandleClientGroupShieldGroupRequest(ttnetlib::CImPdu* pPdu, ttconnection::CMsgConn* pFromConn);

  void HandleGroupShieldGroupResponse(ttnetlib::CImPdu* pPdu);
  void HandleGroupGetShieldByGroupResponse(ttnetlib::CImPdu* pPdu);

 private:
  CGroupChat() {}  // for singleton;

  void _SendPduToUser(ttnetlib::CImPdu* pPdu, uint32_t user_id, ttconnection::CMsgConn* pReqConn = nullptr);

 private:
  static CGroupChat* s_group_chat_instance;

  group_map_t m_group_map;
};

}  // namespace teamtalk::msg_server::service::chat_handler

#endif  // TEAMTALK_MSG_SERVER_SERVICE_CHAT_HANDLER_GROUP_CHAT_H_