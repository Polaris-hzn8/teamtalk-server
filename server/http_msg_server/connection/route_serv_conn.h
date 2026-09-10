/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: RouteServConn.h
 Update Time: Thu 15 Jun 2023 00:43:10 CST
 brief:
*/

#ifndef TEAMTALK_HTTP_SERVER_CONNECTION_ROUTE_SERV_CONN_H_
#define TEAMTALK_HTTP_SERVER_CONNECTION_ROUTE_SERV_CONN_H_

#include <teamtalk/imcore/netlib/core/im_conn.h>
#include <teamtalk/sbase/server_info/server_info.h>

namespace teamtalk::http_server::connection {

namespace ttnetlib = teamtalk::imcore::netlib;
namespace ttserverinfo = teamtalk::sbase::server_info;

class CRouteServConn : public ttnetlib::CImConn {
 public:
  CRouteServConn();
  virtual ~CRouteServConn();

  bool IsOpen() { return m_bOpen; }
  uint64_t GetConnectTime() { return m_connect_time; }

  void Connect(const char* server_ip, uint16_t server_port, uint32_t serv_idx);
  virtual void Close();

  virtual void OnConfirm();
  virtual void OnClose();
  virtual void OnTimer(uint64_t curr_tick);

  virtual void HandlePdu(ttnetlib::CImPdu* pPdu);

 private:
  bool m_bOpen;
  uint32_t m_serv_idx;
  uint64_t m_connect_time;
};

void init_route_serv_conn(ttserverinfo::serv_info_t* server_list, uint32_t server_count);
bool is_route_server_available();
void send_to_all_route_server(ttnetlib::CImPdu* pPdu);
CRouteServConn* get_route_serv_conn();

}  // namespace teamtalk::http_server::connection

#endif  // TEAMTALK_HTTP_SERVER_CONNECTION_ROUTE_SERV_CONN_H_