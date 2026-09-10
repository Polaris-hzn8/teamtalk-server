/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: DBServConn.h
 Update Time: Thu 15 Jun 2023 00:42:06 CST
 brief:
*/

#ifndef TEAMTALK_HTTP_SERVER_CONNECTION_DB_SERV_CONN_H_
#define TEAMTALK_HTTP_SERVER_CONNECTION_DB_SERV_CONN_H_

#include <teamtalk/imcore/netlib/core/im_conn.h>
#include <teamtalk/sbase/server_info/server_info.h>

#include "connection/route_serv_conn.h"

namespace teamtalk::http_server::connection {

namespace ttnetlib = teamtalk::imcore::netlib;
namespace ttserverinfo = teamtalk::sbase::server_info;

class CDBServConn : public ttnetlib::CImConn {
 public:
  CDBServConn();
  virtual ~CDBServConn();

  bool IsOpen() { return m_bOpen; }

  void Connect(const char* server_ip, uint16_t server_port, uint32_t serv_idx);
  virtual void Close();

  virtual void OnConfirm();
  virtual void OnClose();
  virtual void OnTimer(uint64_t curr_tick);

  virtual void HandlePdu(ttnetlib::CImPdu* pPdu);

 private:
  void _HandleStopReceivePacket(ttnetlib::CImPdu* pPdu);
  void _HandleCreateGroupRsp(ttnetlib::CImPdu* pPdu);
  void _HandleChangeMemberRsp(ttnetlib::CImPdu* pPdu);

 private:
  bool m_bOpen;
  uint32_t m_serv_idx;
};

void init_db_serv_conn(ttserverinfo::serv_info_t* server_list, uint32_t server_count, uint32_t concur_conn_cnt);

CDBServConn* get_db_serv_conn_for_login();
CDBServConn* get_db_serv_conn();

}  // namespace teamtalk::http_server::connection

#endif  // TEAMTALK_HTTP_SERVER_CONNECTION_DB_SERV_CONN_H_