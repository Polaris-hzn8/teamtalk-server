/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ImUserManager.h
 Update Time: Thu 15 Jun 2023 00:55:30 CST
 brief:
*/

#ifndef TEAMTALK_MSG_SERVER_DOMAIN_USER_IM_USER_MANAGER_H_
#define TEAMTALK_MSG_SERVER_DOMAIN_USER_IM_USER_MANAGER_H_

#include "domain/user/im_user.h"

namespace teamtalk::msg_server::domain::user {

class CImUserManager {
 public:
  CImUserManager() {}
  ~CImUserManager();

  static CImUserManager* GetInstance();
  CImUser* GetImUserById(uint32_t user_id);
  CImUser* GetImUserByLoginName(std::string login_name);

  ttconnection::CMsgConn* GetMsgConnByHandle(uint32_t user_id, uint32_t handle);
  bool AddImUserByLoginName(std::string login_name, CImUser* pUser);
  void RemoveImUserByLoginName(std::string login_name);

  bool AddImUserById(uint32_t user_id, CImUser* pUser);
  void RemoveImUserById(uint32_t user_id);

  void RemoveImUser(CImUser* pUser);

  void RemoveAll();
  void GetOnlineUserInfo(std::list<user_stat_t>* online_user_info);
  void GetUserConnCnt(std::list<user_conn_t>* user_conn_list, uint32_t& total_conn_cnt);

  void BroadcastPdu(ttnetlib::CImPdu* pdu, uint32_t client_type_flag);

 private:
  ImUserMap_t m_im_user_map;
  ImUserMapByName_t m_im_user_map_by_name;
};

void get_online_user_info(std::list<user_stat_t>* online_user_info);

}  // namespace teamtalk::msg_server::domain::user

#endif  // TEAMTALK_MSG_SERVER_DOMAIN_USER_IM_USER_MANAGER_H_