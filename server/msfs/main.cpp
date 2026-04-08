/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: main.cpp
 Update Time: Thu 15 Jun 2023 00:52:07 CST
 brief:
*/

#include <signal.h>
#include <iostream>
#include "config_file_reader.h"
#include "file_manager.h"
#include "http_conn.h"
#include "netlib.h"
#include "thread_pool.h"

using namespace std;
using namespace msfs;

FileManager* FileManager::m_instance = NULL;
FileManager* g_fileManager = NULL;
CConfigFileReader config_file("msfs.conf");
CThreadPool g_PostThreadPool;
CThreadPool g_GetThreadPool;

void closeall(int fd) {
  int fdlimit = sysconf(_SC_OPEN_MAX);
  while (fd < fdlimit)
    close(fd++);
}

int daemon(int nochdir, int noclose, int asroot) {
  switch (fork()) {
    case 0:
      break;
    case -1:
      return -1;
    default:
      _exit(0); /* exit the original process */
  }

  if (setsid() < 0) /* shoudn't fail */
    return -1;

  if (!asroot && (setuid(1) < 0)) /* shoudn't fail */
    return -1;

  /* dyke out this switch if you want to acquire a control tty in */
  /* the future -- not normally advisable for daemons */

  switch (fork()) {
    case 0:
      break;
    case -1:
      return -1;
    default:
      _exit(0);
  }

  if (!nochdir)
    chdir("/");

  if (!noclose) {
    closeall(0);
    dup(0);
    dup(0);
  }

  return 0;
}

// for client connect in
void http_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam) {
  if (msg == NETLIB_MSG_CONNECT) {
    CHttpConn* pConn = new CHttpConn();
    // CHttpTask* pTask = new CHttpTask(handle, pConn);
    // g_ThreadPool.AddTask(pTask);
    pConn->OnConnect(handle);
  } else {
    log_info("!!!error msg: %d", msg);
  }
}

void doQuitJob() {
  char fileCntBuf[20] = {0};
  snprintf(fileCntBuf, 20, "%llu", g_fileManager->getFileCntCurr());
  config_file.SetConfigValue("FileCnt", fileCntBuf);
  FileManager::destroyInstance();
  netlib_destroy();
  log_info("I'm ready quit...");
}

void Stop(int signo) {
  log_info("receive signal:%d", signo);
  switch (signo) {
    case SIGINT:
    case SIGTERM:
    case SIGQUIT:
      doQuitJob();
      _exit(0);
      break;
    default:
      cout << "unknown signal" << endl;
      _exit(0);
  }
}

int main(int argc, char* argv[]) {
  for (int i = 0; i < argc; ++i) {
    if (strncmp(argv[i], "-d", 2) == 0) {
      if (daemon(1, 0, 1) < 0) {
        cout << "daemon error" << endl;
        return -1;
      }
      break;
    }
  }
  log_info("MsgServer max files can open: %d", getdtablesize());

  std::string listen_ip = config_file.GetConfigValue("ListenIP");
  std::string str_listen_port = config_file.GetConfigValue("ListenPort");
  std::string base_dir = config_file.GetConfigValue("BaseDir");
  std::string str_file_cnt = config_file.GetConfigValue("FileCnt");
  std::string str_files_per_dir = config_file.GetConfigValue("FilesPerDir");
  std::string str_post_thread_count = config_file.GetConfigValue("PostThreadCount");
  std::string str_get_thread_count = config_file.GetConfigValue("GetThreadCount");

  if (listen_ip.empty() || str_listen_port.empty() || base_dir.empty() || str_file_cnt.empty() ||
      str_files_per_dir.empty() || str_post_thread_count.empty() || str_get_thread_count.empty()) {
    log_info("config file miss, exit...");
    return -1;
  }

  log_info("%s,%s", listen_ip.c_str(), str_listen_port.c_str());
  uint16_t listen_port = config_file.GetUint32Value("ListenPort", 0);
  long long int fileCnt = config_file.GetIntValue("FileCnt", 0);
  int filesPerDir = config_file.GetIntValue("FilesPerDir", 0);
  int nPostThreadCount = config_file.GetIntValue("PostThreadCount", 0);
  int nGetThreadCount = config_file.GetIntValue("GetThreadCount", 0);
  if (nPostThreadCount <= 0 || nGetThreadCount <= 0) {
    log_info("thread count is invalied");
    return -1;
  }
  g_PostThreadPool.Init(nPostThreadCount);
  g_GetThreadPool.Init(nGetThreadCount);

  g_fileManager = FileManager::getInstance(listen_ip.c_str(), base_dir.c_str(), fileCnt, filesPerDir);
  int ret = g_fileManager->initDir();
  if (ret) {
    printf("The BaseDir is set incorrectly :%s\n", base_dir.c_str());
    return ret;
  }
  ret = netlib_init();
  if (ret == NETLIB_ERROR)
    return ret;

  CStrExplode listen_ip_list(listen_ip.c_str(), ';');
  for (uint32_t i = 0; i < listen_ip_list.GetItemCnt(); i++) {
    ret = netlib_listen(listen_ip_list.GetItem(i), listen_port, http_callback, NULL);
    if (ret == NETLIB_ERROR)
      return ret;
  }

  signal(SIGINT, Stop);
  signal(SIGTERM, Stop);
  signal(SIGQUIT, Stop);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGHUP, SIG_IGN);

  printf("server start listen on: %s:%d\n", listen_ip.c_str(), listen_port);
  init_http_conn();
  printf("now enter the event loop...\n");

  writePid();
  netlib_eventloop();
  return 0;
}
