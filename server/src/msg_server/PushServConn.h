/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: PushServConn.h
 Update Time: Thu 15 Jun 2023 00:57:12 CST
 brief:
*/

#ifndef PushServConn_H
#define PushServConn_H

#include <iostream>

#include "ServInfo.h"
#include "imconn.h"

class CPushServConn : public CImConn {
public:
    CPushServConn();
    virtual ~CPushServConn();

    bool IsOpen() { return m_bOpen; }

    void Connect(const char* server_ip, uint16_t server_port, uint32_t serv_idx);
    virtual void Close();

    virtual void OnConfirm();
    virtual void OnClose();
    virtual void OnTimer(uint64_t curr_tick);

    virtual void HandlePdu(CImPdu* pPdu);

private:
    void _HandlePushToUserResponse(CImPdu* pPdu);

private:
    bool m_bOpen;
    uint32_t m_serv_idx;
};

void init_push_serv_conn(serv_info_t* server_list, uint32_t server_count);
CPushServConn* get_push_serv_conn();

void build_ios_push_flash(string& flash, uint32_t msg_type, uint32_t from_id);
#endif /* defined(__im_server_TT__PushServConn__) */
