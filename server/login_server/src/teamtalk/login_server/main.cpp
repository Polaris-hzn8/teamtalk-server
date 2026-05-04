/**
 * @author: luochenhao
 * @email: lch2022fox@163.com
 * @time: Mon 04 May 2026 19:37:54 CST
 * @brief: 登录服务主函数
*/

#include <cstring>
#include <string>
#include <version.h>
#include <teamtalk/login_server/http_conn.h>
#include <teamtalk/login_server/login_conn.h>
#include <teamtalk/login_server/common/ip_parser.h>
#include <teamtalk/login_server/server_config/server_config.h>
#include <teamtalk/imcore/common/tools.h>
#include <teamtalk/imcore/string/str_explode.h>
#include <teamtalk/imcore/netlib/core/netlib.h>

namespace ttcommon = teamtalk::imcore::common;
namespace ttnetlib = teamtalk::imcore::netlib;
namespace ttserver = teamtalk::login_server;
namespace ttstr = teamtalk::imcore::string;

// 客户端连接请求事件
void client_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam) {
  if (msg == ttnetlib::NETLIB_MSG_CONNECT) {
    ttserver::CLoginConn* pConn = new ttserver::CLoginConn();
    pConn->OnConnect2(handle, ttserver::LOGIN_CONN_TYPE_CLIENT);
  } else {
    log_error("error client connection request msg: %d ", msg);
  }
}

// 回调函数将被imconn_callback()替换在OnConnect()中，处理连接事件
// MsgServer连接请求事件
void msg_serv_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam) {
  log_info("msg_server connection request");
  if (msg == ttnetlib::NETLIB_MSG_CONNECT) {
    ttserver::CLoginConn* pConn = new ttserver::CLoginConn();
    pConn->OnConnect2(handle, ttserver::LOGIN_CONN_TYPE_MSG_SERV);
  } else {
    log_error("error msg server connection request msg: %d ", msg);
  }
}

// 客户端HTTP连接请求事件
void http_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam) {
  if (msg == ttnetlib::NETLIB_MSG_CONNECT) {
    // 连接被关闭时会自动释放内存delete this
    ttserver::CHttpConn* pConn = new ttserver::CHttpConn();
    pConn->OnConnect(handle);
  } else {
    log_error("error http connection request msg: %d ", msg);
  }
}

int main(int argc, char* argv[]) {
  if ((argc == 2) && (strcmp(argv[1], "-v") == 0)) {
    log_fatal("Server Version: LoginServer/%s\n", VERSION);
    log_fatal("Server Build: %s %s\n", __DATE__, __TIME__);
    return 0;
  }

  signal(SIGPIPE, SIG_IGN);

  // 加载配置文件
  auto& cfg = ttserver::LoginServerConfig::Instance();
  if (!cfg.LoadFromFile("login_server.conf")) {
    log_info("config item missing, exit... ");
    return -1;
  }

  // 初始化网络库
  int ret = ttnetlib::netlib_init();
  if (ret != ttnetlib::NETLIB_OK) {
    log_error("netlib_init failed: %d", ret);
    return ret;
  }

  // 监听客户端连接
  ttstr::CStrExplode client_listen_ip_list(cfg.client_listen_ip().c_str(), ';');
  for (uint32_t i = 0; i < client_listen_ip_list.GetItemCnt(); i++) {
    ret = ttnetlib::netlib_listen(client_listen_ip_list.GetItem(i), cfg.client_port(), client_callback, nullptr);
    if (ret == ttnetlib::NETLIB_ERROR)
      return ret;
  }

  // 监听连接msg_server
  ttstr::CStrExplode msg_server_listen_ip_list(cfg.msg_server_listen_ip().c_str(), ';');
  for (uint32_t i = 0; i < msg_server_listen_ip_list.GetItemCnt(); i++) {
    ret = ttnetlib::netlib_listen(
      msg_server_listen_ip_list.GetItem(i), cfg.msg_server_port(), msg_serv_callback, nullptr);
    if (ret == ttnetlib::NETLIB_ERROR)
      return ret;
  }

  // 监听HTTP连接
  ttstr::CStrExplode http_listen_ip_list(cfg.http_listen_ip().c_str(), ';');
  for (uint32_t i = 0; i < http_listen_ip_list.GetItemCnt(); i++) {
    ret = ttnetlib::netlib_listen(http_listen_ip_list.GetItem(i), cfg.http_port(), http_callback, nullptr);
    if (ret == ttnetlib::NETLIB_ERROR)
      return ret;
  }

  log_info(
    "server start listen on:\nFor client %s:%d\nFor MsgServer: %s:%d\nFor http:%s:%d\n",
    cfg.client_listen_ip().c_str(), cfg.client_port(),
    cfg.msg_server_listen_ip().c_str(), cfg.msg_server_port(),
    cfg.http_listen_ip().c_str(), cfg.http_port());

  // 初始化登录连接
  init_login_conn();
  // 初始化HTTP连接
  init_http_conn();

  // 进入事件循环
  log_info("now enter the event loop...\n");
  ttcommon::write_pid();
  ttnetlib::netlib_eventloop();
  return 0;
}
