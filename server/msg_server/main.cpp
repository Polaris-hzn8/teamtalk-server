/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: msg_server.cpp
 Update Time: Thu 15 Jun 2023 00:56:45 CST
 brief:
*/

#include <teamtalk/sbase/version.h>
#include <teamtalk/sbase/global_define.h>
#include <teamtalk/imcore/netlib/core/netlib.h>

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

serv_info_t* to_serv_info_array(const std::vector<ServerEndpoint>& endpoints) {
  if (endpoints.empty()) {
    return nullptr;
  }
  serv_info_t* arr = new serv_info_t[endpoints.size()];
  for (size_t i = 0; i < endpoints.size(); i++) {
    arr[i].server_ip = endpoints[i].first;
    arr[i].server_port = endpoints[i].second;
  }
  return arr;
}

}  // namespace

// for client connect in
void msg_serv_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam) {
  if (msg == NETLIB_MSG_CONNECT) {
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

  int ret = netlib_init();
  if (ret == NETLIB_ERROR)
    return ret;

  for (const auto& addr : cfg.listen_addresses()) {
    ret = netlib_listen(addr.c_str(), cfg.listen_port(), msg_serv_callback, NULL);
    if (ret == NETLIB_ERROR)
      return ret;
  }

  printf("server start listen on: %s:%d\n", cfg.listen_addresses().front().c_str(), cfg.listen_port());

  init_msg_conn();

  uint32_t file_cnt = cfg.file_servers().size();
  serv_info_t* file_list = to_serv_info_array(cfg.file_servers());
  init_file_serv_conn(file_list, file_cnt);

  uint32_t db_cnt = cfg.expanded_db_servers().size();
  serv_info_t* db_list = to_serv_info_array(cfg.expanded_db_servers());
  init_db_serv_conn(db_list, db_cnt, cfg.concurrent_db_conn_cnt());

  uint32_t login_cnt = cfg.login_servers().size();
  serv_info_t* login_list = to_serv_info_array(cfg.login_servers());
  init_login_serv_conn(login_list, login_cnt, cfg.ip_addr1().c_str(), cfg.ip_addr2().c_str(), cfg.listen_port(), cfg.max_conn_cnt());

  uint32_t route_cnt = cfg.route_servers().size();
  serv_info_t* route_list = to_serv_info_array(cfg.route_servers());
  init_route_serv_conn(route_list, route_cnt);

  uint32_t push_cnt = cfg.push_servers().size();
  serv_info_t* push_list = to_serv_info_array(cfg.push_servers());
  init_push_serv_conn(push_list, push_cnt);

  printf("now enter the event loop...\n");

  writePid();

  netlib_eventloop();

  return 0;
}