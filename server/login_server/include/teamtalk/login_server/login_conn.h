/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: LoginConn.h
 Update Time: Thu 15 Jun 2023 00:45:26 CST
 brief:
*/

#ifndef TEAMTALK_LOGIN_SERVER_LOGIN_CONN_H_
#define TEAMTALK_LOGIN_SERVER_LOGIN_CONN_H_

#include <teamtalk/imcore/netlib/imconn/conn.h>

namespace teamtalk::login_server {

enum {
  LOGIN_CONN_TYPE_CLIENT = 1,
  LOGIN_CONN_TYPE_MSG_SERV = 2
};

typedef struct {
  std::string ip_addr1;   // 电信IP
  std::string ip_addr2;   // 网通IP
  uint16_t port;          // 端口
  uint32_t max_conn_cnt;  // 最大连接数
  uint32_t cur_conn_cnt;  // 当前连接数
  std::string hostname;   // 消息服务器的主机名
} msg_serv_info_t;

class CLoginConn : public teamtalk::imcore::netlib::CImConn {
 public:
  CLoginConn();
  virtual ~CLoginConn();

  virtual void Close();

  void OnConnect2(net_handle_t handle, int conn_type);

  virtual void OnClose();
  virtual void OnTimer(uint64_t curr_tick);

  virtual void HandlePdu(CImPdu* pPdu);

 private:
  void _HandleMsgServInfo(CImPdu* pPdu);     //处理消息服务信息
  void _HandleMsgServRequest(CImPdu* pPdu);  //处理消息服务请求
  void _HandleUserCntUpdate(CImPdu* pPdu);   //处理用户数量更新

 private:
  uint8_t m_conn_type;
};

void init_login_conn();

}  // namespace teamtalk::login_server

#endif  // TEAMTALK_LOGIN_SERVER_LOGIN_CONN_H_
