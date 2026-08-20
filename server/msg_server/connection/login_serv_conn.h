/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: LoginServConn.h
 Update Time: Thu 15 Jun 2023 00:56:39 CST
 brief:
*/

#ifndef TEAMTALK_MSG_SERVER_CONNECTION_LOGIN_SERV_CONN_H_
#define TEAMTALK_MSG_SERVER_CONNECTION_LOGIN_SERV_CONN_H_

#include <teamtalk/imcore/netlib/core/im_conn.h>
#include <teamtalk/sbase/server_info/server_info.h>

namespace teamtalk::msg_server::connection {

namespace ttnetlib = teamtalk::imcore::netlib;
namespace ttserverinfo = teamtalk::sbase::server_info;

class CLoginServConn : public ttnetlib::CImConn {
 public:
  CLoginServConn();
  virtual ~CLoginServConn();

  bool IsOpen() { return m_bOpen; }

  void Connect(const char* server_ip, uint16_t server_port, uint32_t serv_idx);
  virtual void Close();

  virtual void OnConfirm();
  virtual void OnClose();
  virtual void OnTimer(uint64_t curr_tick);

  virtual void HandlePdu(ttnetlib::CImPdu* pPdu);

 private:
  bool m_bOpen;
  uint32_t m_serv_idx;
};

void init_login_serv_conn(ttserverinfo::serv_info_t* server_list,
                          uint32_t server_count,
                          const char* msg_server_ip_addr1,
                          const char* msg_server_ip_addr2,
                          uint16_t msg_server_port,
                          uint32_t max_conn_cnt);

bool is_login_server_available();
void send_to_all_login_server(ttnetlib::CImPdu* pPdu);

}  // namespace teamtalk::msg_server::connection

#endif  // TEAMTALK_MSG_SERVER_CONNECTION_LOGIN_SERV_CONN_H_