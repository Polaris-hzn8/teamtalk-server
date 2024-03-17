/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: LoginConn.h
 Update Time: Thu 15 Jun 2023 00:45:26 CST
 brief:
*/

#ifndef LOGINCONN_H_
#define LOGINCONN_H_

#include "imconn.h"

enum {
    LOGIN_CONN_TYPE_CLIENT = 1,
    LOGIN_CONN_TYPE_MSG_SERV
};

typedef struct {
    string ip_addr1;        // 电信IP
    string ip_addr2;        // 网通IP
    uint16_t port;          // 端口
    uint32_t max_conn_cnt;  // 最大连接数
    uint32_t cur_conn_cnt;  // 当前连接数
    string hostname;        // 消息服务器的主机名
} msg_serv_info_t;

class CLoginConn : public CImConn {
public:
    CLoginConn();
    virtual ~CLoginConn();

    //关闭连接
    virtual void Close();

    //处理连接建立
    void OnConnect2(net_handle_t handle, int conn_type);
    //连接关闭的虚函数
    virtual void OnClose();
    //定时器处理的虚函数
    virtual void OnTimer(uint64_t curr_tick);

    //处理收到的消息包的虚函数
    virtual void HandlePdu(CImPdu* pPdu);

private:
    //处理消息服务信息
    void _HandleMsgServInfo(CImPdu* pPdu);
    //处理用户数量更新
    void _HandleUserCntUpdate(CImPdu* pPdu);
    //处理消息服务请求
    void _HandleMsgServRequest(CImPdu* pPdu);

private:
    int m_conn_type;//连接类型
};

void init_login_conn();

#endif
