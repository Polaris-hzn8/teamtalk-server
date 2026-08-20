/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: FileServConn.h
 Update Time: Thu 15 Jun 2023 00:54:59 CST
 brief:
*/

#ifndef TEAMTALK_MSG_SERVER_CONNECTION_FILE_SERV_CONN_H_
#define TEAMTALK_MSG_SERVER_CONNECTION_FILE_SERV_CONN_H_

#include <iostream>
#include <teamtalk/imcore/ttidl/base_define.pb.h>
#include <teamtalk/imcore/netlib/core/base_socket.h>
#include <teamtalk/imcore/netlib/core/im_conn.h>
#include <teamtalk/sbase/server_info/server_info.h>

namespace teamtalk::msg_server::connection {

namespace ttnetlib = teamtalk::imcore::netlib;
namespace ttserverinfo = teamtalk::sbase::server_info;
namespace ttidlbase = teamtalk::imcore::ttidl::base_define;

class CFileServConn : public ttnetlib::CImConn {
 public:
  CFileServConn();
  virtual ~CFileServConn();

  bool IsOpen() { return m_bOpen; }

  void Connect(const char* server_ip, uint16_t server_port, uint32_t serv_idx);
  virtual void Close();

  virtual void OnConfirm();
  virtual void OnClose();
  virtual void OnTimer(uint64_t curr_tick);

  virtual void HandlePdu(ttnetlib::CImPdu* pPdu);

  const std::list<ttidlbase::IpAddr>* GetFileServerIPList() { return &m_ip_list; }

 private:
  void _HandleFileMsgTransRsp(ttnetlib::CImPdu* pPdu);
  void _HandleFileServerIPRsp(ttnetlib::CImPdu* pPdu);

 private:
  bool m_bOpen;
  uint32_t m_serv_idx;
  uint64_t m_connect_time;
  std::list<ttidlbase::IpAddr> m_ip_list;
};

CFileServConn* get_random_file_serv_conn();
void init_file_serv_conn(ttserverinfo::serv_info_t* server_list, uint32_t server_count);
bool is_file_server_available();

}  // namespace teamtalk::msg_server::connection

#endif  // TEAMTALK_MSG_SERVER_CONNECTION_FILE_SERV_CONN_H_