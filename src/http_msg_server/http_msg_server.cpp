/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: http_msg_server.cpp
 Update Time: Thu 15 Jun 2023 00:42:12 CST
 brief:
*/

#include "config_file_reader.h"
#include "serv_info.h"
#include "netlib.h"
#include "util.h"
#include "version.h"

#include "db_serv_conn.h"
#include "http_conn.h"
#include "http_query.h"
#include "route_serv_conn.h"

#define DEFAULT_CONCURRENT_DB_CONN_CNT 2

// for client connect in
void http_callback(void* callback_data, uint8_t msg, uint32_t handle,
                   void* pParam) {
  if (msg == NETLIB_MSG_CONNECT) {
    CHttpConn* pConn = new CHttpConn();
    pConn->OnConnect(handle);
  } else {
    log_info("!!!error msg: %d ", msg);
  }
}

int main(int argc, char* argv[]) {
  if ((argc == 2) && (strcmp(argv[1], "-v") == 0)) {
    printf("Server Version: HttpMsgServer/%s\n", VERSION);
    printf("Server Build: %s %s\n", __DATE__, __TIME__);
    return 0;
  }

  signal(SIGPIPE, SIG_IGN);
  srand(time(NULL));

  log_info("MsgServer max files can open: %d ", getdtablesize());

  CConfigFileReader config_file("http_msg_server.conf");

  // http服务监听ip端口
  char* listen_ip = config_file.GetConfigName("ListenIP");
  char* str_listen_port = config_file.GetConfigName("ListenPort");

  // DBServer
  // 读取数据库服务连接设置
  uint32_t db_server_count = 0;
  serv_info_t* db_server_list = read_server_config(
      &config_file, "DBServerIP", "DBServerPort", db_server_count);

  // RouteServer
  // 读取路由服务连接设置
  uint32_t route_server_count = 0;
  serv_info_t* route_server_list = read_server_config(
      &config_file, "RouteServerIP", "RouteServerPort", route_server_count);

  // 读取并发连接设置
  uint32_t concurrent_db_conn_cnt = DEFAULT_CONCURRENT_DB_CONN_CNT;
  char* concurrent_db_conn = config_file.GetConfigName("ConcurrentDBConnCnt");
  if (concurrent_db_conn) {
    concurrent_db_conn_cnt = atoi(concurrent_db_conn);
  }

  // 计算总连接数量(用于日志与调试)
  uint32_t expanded_db_conn_cnt = 0;
  if (db_server_count > 0) {
    expanded_db_conn_cnt = db_server_count * concurrent_db_conn_cnt;
    log_info(
        "DB db_server_count: %u concurrent_db_conn_cnt: %u "
        "expanded_db_conn_cnt: %u.\n",
        db_server_count, concurrent_db_conn_cnt, expanded_db_conn_cnt);
  }

  // 创建扩展的服务器列表
  serv_info_t* db_server_list_expanded = NULL;
  if (expanded_db_conn_cnt > 0) {
    db_server_list_expanded = new serv_info_t[expanded_db_conn_cnt];
    for (uint32_t i = 0; i < expanded_db_conn_cnt; i++) {
      uint32_t server_index = i / concurrent_db_conn_cnt;
      db_server_list_expanded[i].server_ip =
          db_server_list[server_index].server_ip.c_str();
      db_server_list_expanded[i].server_port =
          db_server_list[server_index].server_port;
    }
  }

  if (!listen_ip || !str_listen_port) {
    log_info("config file miss, exit... ");
    return -1;
  }

  uint16_t listen_port = atoi(str_listen_port);

  int ret = netlib_init();
  if (ret == NETLIB_ERROR) return ret;

  CStrExplode listen_ip_list(listen_ip, ';');
  for (uint32_t i = 0; i < listen_ip_list.GetItemCnt(); i++) {
    ret = netlib_listen(listen_ip_list.GetItem(i), listen_port, http_callback,
                        NULL);
    if (ret == NETLIB_ERROR) return ret;
  }

  printf("server start listen on: %s:%d\n", listen_ip, listen_port);

  init_http_conn();

  if (db_server_count > 0) {
    HTTP::init_db_serv_conn(db_server_list_expanded, expanded_db_conn_cnt,
                            concurrent_db_conn_cnt);
  }

  if (route_server_count > 0) {
    HTTP::init_route_serv_conn(route_server_list, route_server_count);
  }

  printf("now enter the event loop...\n");

  writePid();

  netlib_eventloop();

  return 0;
}
