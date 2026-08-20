/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: RouteServConn.h
 Update Time: Thu 15 Jun 2023 00:57:22 CST
 brief:
*/

#ifndef TEAMTALK_MSG_SERVER_CONNECTION_ROUTE_SERV_CONN_H_
#define TEAMTALK_MSG_SERVER_CONNECTION_ROUTE_SERV_CONN_H_

#include <teamtalk/imcore/netlib/core/im_conn.h>
#include <teamtalk/sbase/server_info/server_info.h>

namespace teamtalk::msg_server::connection {

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
  void _HandleKickUser(ttnetlib::CImPdu* pPdu);
  void _HandleStatusNotify(ttnetlib::CImPdu* pPdu);
  void _HandleMsgReadNotify(ttnetlib::CImPdu* pPdu);
  void _HandleMsgData(ttnetlib::CImPdu* pPdu);
  void _HandleP2PMsg(ttnetlib::CImPdu* pPdu);
  void _HandleUsersStatusResponse(ttnetlib::CImPdu* pPdu);
  void _HandlePCLoginStatusNotify(ttnetlib::CImPdu* pPdu);
  void _HandleRemoveSessionNotify(ttnetlib::CImPdu* pPdu);
  void _HandleSignInfoChangedNotify(ttnetlib::CImPdu* pPdu);

 private:
  bool m_bOpen;
  uint32_t m_serv_idx;
  uint64_t m_connect_time;
};

void init_route_serv_conn(ttserverinfo::serv_info_t* server_list, uint32_t server_count);
bool is_route_server_available();
void send_to_all_route_server(ttnetlib::CImPdu* pPdu);
CRouteServConn* get_route_serv_conn();

}  // namespace teamtalk::msg_server::connection

#endif  // TEAMTALK_MSG_SERVER_CONNECTION_ROUTE_SERV_CONN_H_