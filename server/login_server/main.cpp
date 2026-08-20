/**
 * @author: luochenhao
 * @email: lch2022fox@163.com
 * @time: Mon 04 May 2026 19:37:54 CST
 * @brief: 登录服务主函数
*/

#include <string>
#include <cstring>

#include <teamtalk/sbase/version.h>
#include <teamtalk/imcore/slog/slog.h>
#include <teamtalk/imcore/common/tools.h>
#include <teamtalk/imcore/netlib/core/netlib.h>

#include "common/ip_parser/ip_parser.h"
#include "common/server_config/server_config.h"
#include "connection/http_conn.h"
#include "connection/login_conn.h"

namespace ttcommon = teamtalk::imcore::common;
namespace ttnetlib = teamtalk::imcore::netlib;
namespace ttconn = teamtalk::login_server::connection;
namespace ttservcfg = teamtalk::login_server::common::server_config;

// 客户端连接请求事件
void client_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam) {
  NOTUSED_ARG(callback_data);
  NOTUSED_ARG(pParam);
  if (msg == ttnetlib::NETLIB_MSG_CONNECT) {
    ttconn::CLoginConn* pConn = new ttconn::CLoginConn();
    pConn->OnConnect2(handle, ttconn::LOGIN_CONN_TYPE_CLIENT);
  } else {
    log_error("error client connection request msg: %d ", msg);
  }
}

// 回调函数将被imconn_callback()替换在OnConnect()中，处理连接事件
// msg_server连接请求事件
void msg_serv_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam) {
  NOTUSED_ARG(callback_data);
  NOTUSED_ARG(pParam);
  log_info("msg_server connection request");
  if (msg == ttnetlib::NETLIB_MSG_CONNECT) {
    ttconn::CLoginConn* pConn = new ttconn::CLoginConn();
    pConn->OnConnect2(handle, ttconn::LOGIN_CONN_TYPE_MSG_SERV);
  } else {
    log_error("error msg server connection request msg: %d ", msg);
  }
}

// 客户端HTTP连接请求事件
void http_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam) {
  NOTUSED_ARG(callback_data);
  NOTUSED_ARG(pParam);
  if (msg == ttnetlib::NETLIB_MSG_CONNECT) {
    // 连接被关闭时会自动释放内存delete this
    ttconn::CHttpConn* pConn = new ttconn::CHttpConn();
    pConn->OnConnect(handle);
  } else {
    log_error("error http connection request msg: %d ", msg);
  }
}

int main(int argc, char* argv[]) {
  // 打印版本信息
  if (argc > 1) {
    const std::string option(argv[1]);
    if (option == "-v" || option == "--version") {
      log_info("Server Version: LoginServer/%s", VERSION);
      log_info("Server Build: %s %s", __DATE__, __TIME__);
      return 0;
    }
  }

  // 忽略SIGPIPE信号
  signal(SIGPIPE, SIG_IGN);

  // 加载配置文件
  auto& cfg = ttservcfg::ServerConfig::Instance();
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
  for (const auto& ip : cfg.client_listen_addresses()) {
    ret = ttnetlib::netlib_listen(ip.c_str(), cfg.client_listen_port(), client_callback, nullptr);
    if (ret == ttnetlib::NETLIB_ERROR)
      return ret;
  }

  // 监听 msg_server
  for (const auto& ip : cfg.msg_server_listen_addresses()) {
    ret = ttnetlib::netlib_listen(ip.c_str(), cfg.msg_server_listen_port(), msg_serv_callback, nullptr);
    if (ret == ttnetlib::NETLIB_ERROR)
      return ret;
  }

  // 监听 HTTP连接
  for (const auto& ip : cfg.http_listen_addresses()) {
    ret = ttnetlib::netlib_listen(ip.c_str(), cfg.http_listen_port(), http_callback, nullptr);
    if (ret == ttnetlib::NETLIB_ERROR)
      return ret;
  }

  log_info(
    "server start listen on:\nFor client %s:%d\nFor MsgServer: %s:%d\nFor http:%s:%d\n",
    cfg.client_listen_addresses().front().c_str(), cfg.client_listen_port(),
    cfg.msg_server_listen_addresses().front().c_str(), cfg.msg_server_listen_port(),
    cfg.http_listen_addresses().front().c_str(), cfg.http_listen_port());

  // 初始化登录连接
  teamtalk::login_server::connection::init_login_conn();
  
  // 初始化HTTP连接
  teamtalk::login_server::connection::init_http_conn();

  // 进入事件循环
  log_info("now enter the event loop...\n");
  ttcommon::write_pid();
  ttnetlib::netlib_eventloop();
  return 0;
}