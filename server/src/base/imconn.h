/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: imconn.h
 Update Time: Mon 12 Jun 2023 23:48:34 CST
 brief: 
*/

#ifndef IMCONN_H_
#define IMCONN_H_

#include "netlib.h"
#include "util.h"
#include "ImPduBase.h"

#define SERVER_HEARTBEAT_INTERVAL	5000
#define SERVER_TIMEOUT				30000
#define CLIENT_HEARTBEAT_INTERVAL	30000
#define CLIENT_TIMEOUT				120000
#define MOBILE_CLIENT_TIMEOUT       60000 * 5
#define READ_BUF_SIZE	2048

/**
 * CImConn 类封装了对网络连接的管理和事件处理
 * 提供了发送和接收数据的功能，并定义了一些虚拟函数供派生类进行重写以实现特定的业务逻辑
 * 
 * CImConn 继承了引用计数类 CRefObject 实现对象的引用计数管理
 * 每个socket tcp长连接都需要绑定一个CImConn对象
 * 要实现具体的业务就需要继承CImConn对象 实现具体的业务逻辑
*/
class CImConn : public CRefObject {
public:
	CImConn();
	virtual ~CImConn();

	//返回连接是否处于忙碌状态
	bool IsBusy() { return m_busy; }
	//发送一个 CImPdu 对象
	int SendPdu(CImPdu* pPdu) { return Send(pPdu->GetBuffer(), pPdu->GetLength()); }
	//发送数据
	int Send(void* data, int len);

	//OnXXX回调函数用于处理连接的不同事件
	//包括连接建立、连接确认、读取数据、写入数据、连接关闭、定时器事件和写入完成事件
	virtual void OnConnect(net_handle_t handle) { m_handle = handle; }
	virtual void OnConfirm() {}
	virtual void OnRead();
	virtual void OnWrite();
	virtual void OnClose() {}
	virtual void OnTimer(uint64_t curr_tick) {}
    virtual void OnWriteCompelete() {};

	//处理接收到的 CImPdu 对象
	virtual void HandlePdu(CImPdu* pPdu) {}

protected:
	net_handle_t	m_handle;//网络句柄，用于标识连接 int类型
	bool			m_busy;//连接是否处于忙碌状态

	string			m_peer_ip;//对端IP地址
	uint16_t		m_peer_port;//对端端口号
	CSimpleBuffer	m_in_buf;//输入缓冲区
	CSimpleBuffer	m_out_buf;//输出缓冲区

	bool			m_policy_conn;//连接是否为策略连接
	uint32_t		m_recv_bytes;//已接收的字节数
	uint64_t		m_last_send_tick;//最后一次发送数据的时间戳
	uint64_t		m_last_recv_tick;//最后一次接收数据的时间戳
    uint64_t        m_last_all_user_tick;//最后一次处理所有用户的时间戳
};

//哈希映射容器类型 用于将网络句柄 net_handle_t 映射到 CImConn* 对象
typedef hash_map<net_handle_t, CImConn*> ConnMap_t;

//哈希映射容器类型 用于将uint32_t类型的用户id 映射到 CImConn* 对象
typedef hash_map<uint32_t, CImConn*> UserMap_t;

//将具体的事件回调委托给对应的连接对象 CImConn 进行处理 根据不同的消息类型 分发事件处理
void imconn_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam);

void ReadPolicyFile();

#endif
