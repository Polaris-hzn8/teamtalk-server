/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: db_proxy_server.cpp
 Update Time: Wed 14 Jun 2023 23:05:27 CST
 brief:
*/

#include "config_file_reader.h"
#include "enc_dec.h"
#include "http_client.h"
#include "netlib.h"
#include "proxy_conn.h"
#include "version.h"

#include "cache_pool.h"
#include "db_pool.h"
#include "sync_center.h"
#include "thread_pool.h"

#include "business/AudioModel.h"
#include "business/FileModel.h"
#include "business/GroupMessageModel.h"
#include "business/GroupModel.h"
#include "business/MessageModel.h"
#include "business/RelationModel.h"
#include "business/SessionModel.h"
#include "business/UserModel.h"

std::string strAudioEnc;
// this callback will be replaced by imconn_callback() in OnConnect()
void proxy_serv_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam) {
  if (msg == NETLIB_MSG_CONNECT) {
    CProxyConn* pConn = new CProxyConn();
    pConn->OnConnect(handle);
  } else {
    log_info("!!!error msg: %d", msg);
  }
}

int main(int argc, char* argv[]) {
  if ((argc == 2) && (strcmp(argv[1], "-v") == 0)) {
    printf("Server Version: DBProxyServer/%s\n", VERSION);
    printf("Server Build: %s %s\n", __DATE__, __TIME__);
    return 0;
  }

  //忽略SIGPIPE信号 以避免在处理网络连接时收到此信号导致进程退出
  signal(SIGPIPE, SIG_IGN);
  srand(time(NULL));

  // CacheManager初始化 Redis相关
  CacheManager* pCacheManager = CacheManager::getInstance();
  if (!pCacheManager) {
    log_info("CacheManager init failed");
    return -1;
  }

  // CDBManager初始化 MySQL相关
  CDBManager* pDBManager = CDBManager::getInstance();
  if (!pDBManager) {
    log_info("DBManager init failed");
    return -1;
  }

  log_info("db init success");

  // 初始化各单例对象
  if (!CAudioModel::getInstance())
    return -1;
  if (!CGroupMessageModel::getInstance())
    return -1;
  if (!CGroupModel::getInstance())
    return -1;
  if (!CMessageModel::getInstance())
    return -1;
  if (!CSessionModel::getInstance())
    return -1;
  if (!CRelationModel::getInstance())
    return -1;
  if (!CUserModel::getInstance())
    return -1;
  if (!CFileModel::getInstance())
    return -1;

  // 服务配置读取
  CConfigFileReader config_file("db_proxy_server.conf");

  std::string listen_ip = config_file.GetConfigValue("ListenIP");
  std::string str_listen_port = config_file.GetConfigValue("ListenPort");
  std::string str_thread_num = config_file.GetConfigValue("ThreadNum");
  std::string str_file_site = config_file.GetConfigValue("MsfsSite");
  std::string str_aes_key = config_file.GetConfigValue("aesKey");

  if (listen_ip.empty() || str_listen_port.empty() || str_thread_num.empty() || str_file_site.empty() ||
      str_aes_key.empty()) {
    log_info("missing ListenIP/ListenPort/ThreadNum/MsfsSite/aesKey, exit...");
    return -1;
  }
  if (str_aes_key.length() != 32) {
    log_info("aes key is invalied");
    return -2;
  }

  uint16_t listen_port = config_file.GetUint32Value("ListenPort", 0);
  uint32_t thread_num = config_file.GetUint32Value("ThreadNum", 0);

  CAudioModel::getInstance()->setUrl(str_file_site);

  // TCP网络连接库支持
  int ret = netlib_init();
  if (NETLIB_ERROR == ret)
    return ret;

  // CURL库初始化-支持HTTP请求
  curl_global_init(CURL_GLOBAL_ALL);

  // 初始化代理连接
  init_proxy_conn(thread_num);

  CSyncCenter::getInstance()->init();
  CSyncCenter::getInstance()->startSync();

  // 监听多个IP地址和端口
  // 根据配置文件中的 ListenIP 和 ListenPort 配置项
  // 使用netlib_listen()函数在每个IP地址和端口上启动监听 并指定回调函数
  // proxy_serv_callback
  CStrExplode listen_ip_list(listen_ip.c_str(), ';');
  for (uint32_t i = 0; i < listen_ip_list.GetItemCnt(); i++) {
    ret = netlib_listen(listen_ip_list.GetItem(i), listen_port, proxy_serv_callback, NULL);
    if (ret == NETLIB_ERROR)
      return ret;
  }

  printf("server start listen on: %s:%d\n", listen_ip.c_str(), listen_port);
  printf("now enter the event loop...\n");
  writePid();

  // 进入事件循环
  netlib_eventloop(10);

  return 0;
}
