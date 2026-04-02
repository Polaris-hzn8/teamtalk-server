/*
 Reviser: Polaris_hzn8
 Email: lch2022fox@163.com
 filename: session_manager.h
 Update Time: Sun 10 Aug 2025 12:14:39 CST
 brief:
*/

#ifndef __my_push_server__session_manager__
#define __my_push_server__session_manager__

#include <stdio.h>
#include <list>
#include <unordered_map>

#include "apns_client.h"
#include "push_server.h"
#include "push_session.h"
#include "socket/base_io_stream.h"
#include "thread/base_thread.hpp"
#include "timer/Timer.hpp"

class CSessionManager {
 public:
  CSessionManager();
  virtual ~CSessionManager();

  static CSessionManager* GetInstance();
  static void TimerProc(int32_t nIndex, void* param);

  void StartCheckPushSession();
  void StopCheckPushSession();
  void CheckPushSessionTimeOut();
  void CheckPushSessionDelete();

  void AddPushSessionBySockID(uint32_t nsockid, push_session_ptr pSession);
  void RemovePushSessionBySockID(uint32_t nsockid);
  push_session_ptr GetPushSessionBySockID(uint32_t nsockid);
  void ClearPushSession();
  void StopAllPushSession();

  void SetAPNSClient(apns_client_ptr pClient) { m_pAPNSClient = pClient; }
  apns_client_ptr GetAPNSClient() { return m_pAPNSClient; }
  void RemoveAPNSClient();

  void SetPushServer(push_server_ptr pServer) { m_pPushServer = pServer; }
  push_server_ptr GetPushServer() { return m_pPushServer; }
  void RemovePushServer();

 private:
  void _ClearPushSessionForMap();

 private:
  CBaseMutex m_MapIOPushSessionBySockIDMutex;
  apns_client_ptr m_pAPNSClient;
  push_server_ptr m_pPushServer;
  CTimer m_checktimer;
  std::unordered_map<uint32_t, push_session_ptr> m_MapPushSessionBySockID;
};

#endif /* defined(__my_push_server__session_manager__) */
