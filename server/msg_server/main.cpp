/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: msg_server.cpp
 Update Time: Thu 15 Jun 2023 00:56:45 CST
 brief:
*/

#include <teamtalk/imcore/common/tools.h>
#include <teamtalk/sbase/version.h>
#include <teamtalk/sbase/global_define.h>
#include <teamtalk/imcore/netlib/core/netlib.h>
#include <teamtalk/imcore/slog/slog.h>

#include "common/server_config/server_config.h"
#include "connection/msg_conn.h"
#include "connection/db_serv_conn.h"
#include "connection/file_serv_conn.h"
#include "connection/push_serv_conn.h"
#include "connection/login_serv_conn.h"
#include "connection/route_serv_conn.h"

namespace {

using teamtalk::msg_server::common::server_config::ServerConfig;
using teamtalk::msg_server::common::server_config::ServerEndpoint;

using teamtalk::msg_server::connection::CMsgConn;
using teamtalk::msg_server::connection::init_msg_conn;
using teamtalk::msg_server::connection::init_file_serv_conn;
using teamtalk::msg_server::connection::init_db_serv_conn;
using teamtalk::msg_server::connection::init_login_serv_conn;
using teamtalk::msg_server::connection::init_route_serv_conn;
using teamtalk::msg_server::connection::init_push_serv_conn;

namespace ttserverinfo = teamtalk::sbase::server_info;
namespace ttnetlib = teamtalk::imcore::netlib;
namespace ttcommon = teamtalk::imcore::common;

ttserverinfo::serv_info_t* to_serv_info_array(const std::vector<ServerEndpoint>& endpoints) {
  if (endpoints.empty()) {
    return nullptr;
  }
  ttserverinfo::serv_info_t* arr = new ttserverinfo::serv_info_t[endpoints.size()];
  for (size_t i = 0; i < endpoints.size(); i++) {
    arr[i].server_ip = endpoints[i].first;
    arr[i].server_port = endpoints[i].second;
  }
  return arr;
}

}  // namespace

void msg_serv_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam) {
  if (msg == ttnetlib::NETLIB_MSG_CONNECT) {
    CMsgConn* pConn = new CMsgConn();
    pConn->OnConnect(handle);
  } else {
    log_info("!error msg: %d ", msg);
  }
}

int main(int argc, char* argv[]) {
  if ((argc == 2) && (strcmp(argv[1], "-v") == 0)) {
    printf("Server Version: MsgServer/%s\n", VERSION);
    printf("Server Build: %s %s\n", __DATE__, __TIME__);
    return 0;
  }

  signal(SIGPIPE, SIG_IGN);
  srand(time(NULL));

  log_info("MsgServer max files can open: %d ", getdtablesize());

  auto& cfg = ServerConfig::Instance();
  if (!cfg.LoadFromFile("msg_server.conf")) {
    log_info("config file load failed, exit... ");
    return 1;
  }

  int ret = ttnetlib::netlib_init();
  if (ret == ttnetlib::NETLIB_ERROR)
    return ret;

  for (const auto& addr : cfg.listen_addresses()) {
    ret = ttnetlib::netlib_listen(addr.c_str(), cfg.listen_port(), msg_serv_callback, NULL);
    if (ret == ttnetlib::NETLIB_ERROR)
      return ret;
  }

  log_info("server start listen on: %s:%d\n", cfg.listen_addresses().front().c_str(), cfg.listen_port());

  // init msg conn
  init_msg_conn();

  // init file serv conn
  uint32_t file_cnt = cfg.file_servers().size();
  ttserverinfo::serv_info_t* file_list = to_serv_info_array(cfg.file_servers());
  init_file_serv_conn(file_list, file_cnt);

  // init db serv conn
  uint32_t db_cnt = cfg.expanded_db_servers().size();
  ttserverinfo::serv_info_t* db_list = to_serv_info_array(cfg.expanded_db_servers());
  init_db_serv_conn(db_list, db_cnt, cfg.concurrent_db_conn_cnt());

  // init login serv conn
  uint32_t login_cnt = cfg.login_servers().size();
  ttserverinfo::serv_info_t* login_list = to_serv_info_array(cfg.login_servers());
  init_login_serv_conn(login_list, login_cnt, cfg.ip_addr1().c_str(), cfg.ip_addr2().c_str(), cfg.listen_port(), cfg.max_conn_cnt());

  // init route serv conn
  uint32_t route_cnt = cfg.route_servers().size();
  ttserverinfo::serv_info_t* route_list = to_serv_info_array(cfg.route_servers());
  init_route_serv_conn(route_list, route_cnt);

  // init push serv conn
  uint32_t push_cnt = cfg.push_servers().size();
  ttserverinfo::serv_info_t* push_list = to_serv_info_array(cfg.push_servers());
  init_push_serv_conn(push_list, push_cnt);

  log_info("now enter the event loop...\n");

  ttcommon::write_pid();
  ttnetlib::netlib_eventloop();

  return 0;
}