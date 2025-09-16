
/*
 Reviser: Polaris_hzn8
 Email: lch2022fox@163.com
 filename: push_server_handler.h
 Update Time: Sun 10 Aug 2025 12:12:41 CST
 brief: 
*/

#ifndef __my_push_server__push_server_handler__
#define __my_push_server__push_server_handler__

#include <stdio.h>
#include "socket/base_handler.hpp"

class CPushServerHandler : public CBaseHandler
{
public:
    CPushServerHandler() {}
    virtual ~CPushServerHandler() {}
    
    void OnAccept(uint32_t nsockid, S_SOCKET sock, const char* szIP, int32_t nPort);
    void OnClose(uint32_t nsockid);
private:
};

#endif /* defined(__my_push_server__push_server_handler__) */
