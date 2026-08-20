/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: FileHandler.h
 Update Time: Thu 15 Jun 2023 00:54:46 CST
 brief:
*/

#ifndef TEAMTALK_MSG_SERVER_SERVICE_FILE_HANDLER_FILE_HANDLER_H_
#define TEAMTALK_MSG_SERVER_SERVICE_FILE_HANDLER_FILE_HANDLER_H_

#include <teamtalk/imcore/netlib/core/im_pdu.h>

#include "connection/msg_conn.h"

namespace teamtalk::msg_server::service::file_handler {

namespace ttconnection = teamtalk::msg_server::connection;
namespace ttnetlib = teamtalk::imcore::netlib;

class CFileHandler {
 public:
  virtual ~CFileHandler() {}

  static CFileHandler* getInstance();

  void HandleClientFileRequest(ttconnection::CMsgConn* pMsgConn, ttnetlib::CImPdu* pPdu);
  void HandleClientFileHasOfflineReq(ttconnection::CMsgConn* pMsgConn, ttnetlib::CImPdu* pPdu);
  void HandleClientFileAddOfflineReq(ttconnection::CMsgConn* pMsgConn, ttnetlib::CImPdu* pPdu);
  void HandleClientFileDelOfflineReq(ttconnection::CMsgConn* pMsgConn, ttnetlib::CImPdu* pPdu);
  void HandleFileHasOfflineRes(ttnetlib::CImPdu* pPdu);
  void HandleFileNotify(ttnetlib::CImPdu* pPdu);

 private:
  CFileHandler() {}

 private:
  static CFileHandler* s_handler_instance;
};

}  // namespace teamtalk::msg_server::service::file_handler

#endif  // TEAMTALK_MSG_SERVER_SERVICE_FILE_HANDLER_FILE_HANDLER_H_