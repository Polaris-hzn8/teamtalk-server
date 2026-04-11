/*
 Reviser: Polaris_hzn8
 Email: lch2022fox@163.com
 filename: apns_feedback_handler.h
 Update Time: Sun 10 Aug 2025 12:06:55 CST
 brief:
*/

#ifndef __my_push_server__apns_feedback_handler__
#define __my_push_server__apns_feedback_handler__

#include <stdio.h>
#include "apns_msg.h"
#include "socket/base_handler.hpp"

class CAPNSFeedBackHandler : public CBaseHandler {
 public:
  CAPNSFeedBackHandler() {}
  virtual ~CAPNSFeedBackHandler() {}

  virtual void OnException(uint32_t nsockid, int32_t nErrorCode);

  virtual void OnClose(uint32_t nsockid);

  virtual void OnConnect(uint32_t nsockid);

  virtual void OnSSLConnect(uint32_t nsockid);

  virtual void OnRecvData(const char* szBuf, int32_t nBufSize);

 private:
  CAPNSFeedBackResMsg m_Msg;
};

#endif /* defined(__my_push_server__apns_feedback_handler__) */
