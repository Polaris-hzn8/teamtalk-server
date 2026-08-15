/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: LoginConn.h
 Update Time: Thu 15 Jun 2023 00:45:26 CST
 brief:
*/

#ifndef TEAMTALK_LOGIN_SERVER_CONNECTION_LOGIN_CONN_H_
#define TEAMTALK_LOGIN_SERVER_CONNECTION_LOGIN_CONN_H_

#include <teamtalk/imcore/netlib/core/im_conn.h>

namespace teamtalk::login_server::connection {

enum {
  LOGIN_CONN_TYPE_CLIENT = 1,
  LOGIN_CONN_TYPE_MSG_SERV = 2
};

class CLoginConn : public teamtalk::imcore::netlib::CImConn {
 public:
  CLoginConn();
  virtual ~CLoginConn();

  virtual void Close();

  void OnConnect2(net_handle_t handle, int conn_type);

  virtual void OnClose();
  virtual void OnTimer(uint64_t curr_tick);

  virtual void HandlePdu(teamtalk::imcore::netlib::CImPdu* pPdu);

 private:
  void _HandleMsgServInfo(teamtalk::imcore::netlib::CImPdu* pPdu);     //处理消息服务信息
  void _HandleMsgServRequest(teamtalk::imcore::netlib::CImPdu* pPdu);  //处理消息服务请求
  void _HandleUserCntUpdate(teamtalk::imcore::netlib::CImPdu* pPdu);   //处理用户数量更新

 private:
  uint8_t m_conn_type;
};

void init_login_conn();

}  // namespace teamtalk::login_server::connection

#endif  // TEAMTALK_LOGIN_SERVER_CONNECTION_LOGIN_CONN_H_
