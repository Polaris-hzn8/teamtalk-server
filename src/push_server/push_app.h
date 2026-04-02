/*
 Reviser: Polaris_hzn8
 Email: lch2022fox@163.com
 filename: push_app.h
 Update Time: Sun 10 Aug 2025 12:11:00 CST
 brief:
*/

#ifndef __my_push_server__push_app__
#define __my_push_server__push_app__

#include <stdio.h>
#include "socket/epoll_io_loop.h"
#include "type/base_type.h"

class CPushApp {
 public:
  CPushApp();
  virtual ~CPushApp();

  static CPushApp* GetInstance();

  BOOL Init();
  BOOL UnInit();
  BOOL Start();
  BOOL Stop();

  CEpollIOLoop& GetIOLoop() { return m_io; }

 private:
  BOOL m_bInit;
  CEpollIOLoop m_io;
};

#endif /* defined(__my_push_server__push_app__) */
