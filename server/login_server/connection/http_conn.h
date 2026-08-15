/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: HttpConn.h
 Update Time: Thu 15 Jun 2023 00:44:51 CST
 brief:
*/

#ifndef TEAMTALK_LOGIN_SERVER_CONNECTION_HTTP_CONN_H_
#define TEAMTALK_LOGIN_SERVER_CONNECTION_HTTP_CONN_H_

#include <unordered_map>
#include <teamtalk/imcore/common/tools.h>
#include <teamtalk/imcore/common/ref_object.h>
#include <teamtalk/imcore/netlib/core/netlib.h>
#include <teamtalk/imcore/netlib/core/simple_buffer.h>
#include <teamtalk/imcore/http_client/http_parser_wrapper.h>

namespace teamtalk::login_server::connection {

#define HTTP_CONN_TIMEOUT 60000

#define READ_BUF_SIZE 2048
#define HTTP_RESPONSE_HTML \
  "HTTP/1.1 200 OK\r\n"    \
  "Connection:close\r\n"   \
  "Content-Length:%d\r\n"  \
  "Content-Type:text/html;charset=utf-8\r\n\r\n%s"
#define HTTP_RESPONSE_HTML_MAX 1024

enum {
  CONN_STATE_IDLE,
  CONN_STATE_CONNECTED,
  CONN_STATE_OPEN,
  CONN_STATE_CLOSED,
};

class CHttpConn : public teamtalk::imcore::common::CRefObject {
 public:
  CHttpConn();
  virtual ~CHttpConn();

  uint32_t GetConnHandle() { return m_conn_handle; }
  char* GetPeerIP() { return (char*)m_peer_ip.c_str(); }

  int Send(void* data, int len);

  void Close();
  void OnConnect(net_handle_t handle);
  void OnRead();
  void OnWrite();
  void OnClose();
  void OnTimer(uint64_t curr_tick);
  void OnWriteComlete();

 private:
  void _HandleMsgServRequest(std::string& url, std::string& post_data);

 protected:
  net_handle_t m_sock_handle;
  uint32_t m_conn_handle;
  bool m_busy;

  uint32_t m_state;
  std::string m_peer_ip;
  uint16_t m_peer_port;
  
  uint64_t m_last_send_tick;
  uint64_t m_last_recv_tick;

  teamtalk::imcore::netlib::CSimpleBuffer m_in_buf;
  teamtalk::imcore::netlib::CSimpleBuffer m_out_buf;
  teamtalk::imcore::http_client::CHttpParserWrapper m_cHttpParser;
};

typedef std::unordered_map<uint32_t, CHttpConn*> HttpConnMap_t;

CHttpConn* FindHttpConnByHandle(uint32_t handle);

void init_http_conn();

}  // namespace teamtalk::login_server::connection

#endif  // TEAMTALK_LOGIN_SERVER_CONNECTION_HTTP_CONN_H_
