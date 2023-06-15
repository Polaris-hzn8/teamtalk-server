/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: FileServConn.h
 Update Time: Thu 15 Jun 2023 00:54:59 CST
 brief:
*/

#ifndef __FileServConn__
#define __FileServConn__

#include <iostream>

#include "BaseSocket.h"
#include "IM.BaseDefine.pb.h"
#include "ServInfo.h"
#include "imconn.h"
class CFileServConn : public CImConn {
public:
    CFileServConn();
    virtual ~CFileServConn();

    bool IsOpen() { return m_bOpen; }

    void Connect(const char* server_ip, uint16_t server_port, uint32_t serv_idx);
    virtual void Close();

    virtual void OnConfirm();
    virtual void OnClose();
    virtual void OnTimer(uint64_t curr_tick);

    virtual void HandlePdu(CImPdu* pPdu);

    const list<IM::BaseDefine::IpAddr>* GetFileServerIPList() { return &m_ip_list; }

private:
    void _HandleFileMsgTransRsp(CImPdu* pPdu);
    void _HandleFileServerIPRsp(CImPdu* pPdu);

private:
    bool m_bOpen;
    uint32_t m_serv_idx;
    uint64_t m_connect_time;
    list<IM::BaseDefine::IpAddr> m_ip_list;
};

void init_file_serv_conn(serv_info_t* server_list, uint32_t server_count);
bool is_file_server_available();
CFileServConn* get_random_file_serv_conn();
#endif /* defined(__FileServConn__) */
