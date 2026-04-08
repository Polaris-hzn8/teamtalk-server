
/*
 Reviser: Polaris_hzn8
 Email: lch2022fox@163.com
 filename: login_server.cpp
 Update Time: Wed 06 Aug 2025 17:05:24 CST
 brief:
*/

#include "config_file_reader.h"
#include "http_conn.h"
#include "ipparser.h"
#include "login_conn.h"
#include "netlib.h"
#include "server_register.h"
#include "version.h"

IpParser* pIpParser = nullptr;
std::string strMsfsUrl;
std::string strDiscovery;  //发现获取地址
void client_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam) {
  if (msg == NETLIB_MSG_CONNECT) {
    CLoginConn* pConn = new CLoginConn();
    pConn->OnConnect2(handle, LOGIN_CONN_TYPE_CLIENT);
  } else {
    log_error("!!!error msg: %d ", msg);
  }
}

// this callback will be replaced by imconn_callback() in OnConnect()
// msg_server请求连接事件
void msg_serv_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam) {
  log("msg_server come in");

  if (msg == NETLIB_MSG_CONNECT) {
    CLoginConn* pConn = new CLoginConn();
    pConn->OnConnect2(handle, LOGIN_CONN_TYPE_MSG_SERV);  // 绑定fd
  } else {
    log_error("!!!error msg: %d ", msg);
  }
}

// Android、IOS、PC等客户端请求连接事件
void http_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam) {
  if (msg == NETLIB_MSG_CONNECT) {
    // 这里是不是觉得很奇怪,为什么new了对象却没有释放?
    // 实际上对象在被Close时使用delete this的方式释放自己
    CHttpConn* pConn = new CHttpConn();
    pConn->OnConnect(handle);
  } else {
    log_error("!!!error msg: %d ", msg);
  }
}

int main(int argc, char* argv[]) {
  if ((argc == 2) && (strcmp(argv[1], "-v") == 0)) {
    log_fatal("Server Version: LoginServer/%s\n", VERSION);
    log_fatal("Server Build: %s %s\n", __DATE__, __TIME__);
    return 0;
  }

  signal(SIGPIPE, SIG_IGN);

  CConfigFileReader config_file("loginserver.conf");

  std::string client_listen_ip = config_file.GetConfigValue("ClientListenIP");
  std::string str_client_port = config_file.GetConfigValue("ClientPort");
  std::string http_listen_ip = config_file.GetConfigValue("HttpListenIP");
  std::string str_http_port = config_file.GetConfigValue("HttpPort");
  std::string msg_server_listen_ip = config_file.GetConfigValue("MsgServerListenIP");
  std::string str_msg_server_port = config_file.GetConfigValue("MsgServerPort");
  strMsfsUrl = config_file.GetConfigValue("msfs");
  strDiscovery = config_file.GetConfigValue("discovery");

  // etcd注册信息
  std::string str_register_center_ip = config_file.GetConfigValue("RegisterCenterIp");
  std::string str_register_center_port = config_file.GetConfigValue("RegisterCenterPort");
  std::string str_service_id = config_file.GetConfigValue("ServiceId");
  std::string str_service_dir = config_file.GetConfigValue("ServiceDir");
  std::string str_host_ip = config_file.GetConfigValue("HostIp");
  std::string str_register_ttl = config_file.GetConfigValue("RegisterTTL");

  if (msg_server_listen_ip.empty() || str_msg_server_port.empty() || http_listen_ip.empty() || str_http_port.empty() ||
      strMsfsUrl.empty() || strDiscovery.empty()) {
    printf("config item missing, exit... \n");
    log("config item missing, exit... ");
    return -1;
  }

  if (str_register_center_ip.empty() || str_register_center_port.empty() || str_service_id.empty() ||
      str_service_dir.empty() || str_host_ip.empty() || str_register_ttl.empty()) {
    printf("config etcd str_register_center_ip:%s\n", str_register_center_ip.c_str());
    printf("config etcd str_register_center_port:%s\n", str_register_center_port.c_str());
    printf("config etcd str_service_id:%s\n", str_service_id.c_str());
    printf("config etcd str_service_dir:%s\n", str_service_dir.c_str());
    printf("config etcd str_host_ip:%s\n", str_host_ip.c_str());
    printf("config etcd str_register_ttl:%s\n", str_register_ttl.c_str());

    log("config etcd item missing, exit... ");
    return -1;
  }

  uint16_t client_port = config_file.GetUint32Value("ClientPort", 0);
  uint16_t msg_server_port = config_file.GetUint32Value("MsgServerPort", 0);
  uint16_t http_port = config_file.GetUint32Value("HttpPort", 0);

  pIpParser = new IpParser();

  int ret = netlib_init();

  if (ret == NETLIB_ERROR)
    return ret;
  CStrExplode client_listen_ip_list(client_listen_ip.c_str(), ';');
  for (uint32_t i = 0; i < client_listen_ip_list.GetItemCnt(); i++) {
    ret = netlib_listen(client_listen_ip_list.GetItem(i), client_port, client_callback, NULL);
    if (ret == NETLIB_ERROR)
      return ret;
  }

  CStrExplode msg_server_listen_ip_list(msg_server_listen_ip.c_str(), ';');
  for (uint32_t i = 0; i < msg_server_listen_ip_list.GetItemCnt(); i++) {
    ret = netlib_listen(msg_server_listen_ip_list.GetItem(i), msg_server_port, msg_serv_callback, NULL);
    if (ret == NETLIB_ERROR)
      return ret;
  }

  CStrExplode http_listen_ip_list(http_listen_ip.c_str(), ';');
  for (uint32_t i = 0; i < http_listen_ip_list.GetItemCnt(); i++) {
    ret = netlib_listen(http_listen_ip_list.GetItem(i), http_port, http_callback, NULL);
    if (ret == NETLIB_ERROR)
      return ret;
  }

  log(
    "server start listen on:\nFor client %s:%d\nFor MsgServer: %s:%d\nFor "
    "http:%s:%d\n",
    client_listen_ip.c_str(),
    client_port,
    msg_server_listen_ip.c_str(),
    msg_server_port,
    http_listen_ip.c_str(),
    http_port);

  init_login_conn();
  init_http_conn();

  // 注册中心业务
  LoginServerRegInfo reg_info;
  reg_info.reg_center_addr.clear();
  reg_info.reg_center_addr.append(str_register_center_ip);
  reg_info.reg_center_addr.append(":");
  reg_info.reg_center_addr.append(str_register_center_port);
  reg_info.service_id = str_service_id;
  reg_info.service_dir = str_service_dir;
  reg_info.host_ip = str_host_ip;
  reg_info.ttl = config_file.GetUint32Value("RegisterTTL", 30);
  reg_info.client_port = client_port;
  reg_info.http_port = http_port;
  reg_info.msg_port = msg_server_port;

  ret = init_server_register(&reg_info);
  if (ret < 0) {
    printf("init_server_register failed, then exit it");
    log("init_server_register failed, then exit it");
    exit(1);  // 姑且注册不上先退出
  }

  log("now enter the event loop...\n");

  writePid();

  netlib_eventloop();

  return 0;
}
