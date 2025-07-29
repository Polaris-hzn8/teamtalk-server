/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: imconn.h
 Update Time: Mon 12 Jun 2023 23:48:34 CST
 brief: 
*/

#ifndef _IMCONN_H_
#define _IMCONN_H_

#include <unordered_map>
#include "ImPduBase.h"
#include "common/util.h"
#include "netlib/netlib.h"

#define SERVER_HEARTBEAT_INTERVAL	5000
#define SERVER_TIMEOUT				30000
#define CLIENT_HEARTBEAT_INTERVAL	30000
#define CLIENT_TIMEOUT				120000
#define MOBILE_CLIENT_TIMEOUT       60000 * 5
#define READ_BUF_SIZE				2048

/**
 * @brief CImConn
 * 1.提供发送和接收数据的功能
 * 2.封装了对网络连接的管理 和事件处理
 * 3.通过继承CImConn对象 来实现具体的业务逻辑
 * 4.每个TCP长连接都需要绑定一个CImConn对象(Socket)
 */
class CImConn : public CRefObject
{
public:
	CImConn();
	virtual ~CImConn();

	bool IsBusy() { return m_busy; }

	// 数据发送
	int Send(void* data, int len);
	int SendPdu(CImPdu* pPdu) { return Send(pPdu->GetBuffer(), pPdu->GetLength()); }

	// 回调处理连接的事件
	virtual void OnConnect(net_handle_t handle) { m_handle = handle; }	// 连接建立
	virtual void OnConfirm() {}							// 连接确认
	virtual void OnRead();								// 数据读取	
	virtual void OnWrite();								// 数据写入
	virtual void OnClose() {}							// 连接关闭
	virtual void OnTimer(uint64_t curr_tick) {}			// 定时器事件
    virtual void OnWriteCompelete() {};					// 写入完成事件

	// 处理连接接收到的CImPdu对象
	virtual void HandlePdu(CImPdu* pPdu) {}

protected:
	net_handle_t	m_handle;				//网络句柄，用于标识连接 int类型
	bool			m_busy;					//连接是否处于忙碌状态

	std::string		m_peer_ip;				//对端IP地址
	uint16_t		m_peer_port;			//对端端口号

	CSimpleBuffer	m_in_buf;				//输入缓冲区
	CSimpleBuffer	m_out_buf;				//输出缓冲区

	bool			m_policy_conn;			//连接是否为策略连接
	uint32_t		m_recv_bytes;			//已接收的字节数
	uint64_t		m_last_send_tick;		//最后一次发送数据的时间戳
	uint64_t		m_last_recv_tick;		//最后一次接收数据的时间戳
    uint64_t        m_last_all_user_tick;	//最后一次处理所有用户的时间戳
};

// CImConn对象与TCP连接绑定
typedef std::unordered_map<net_handle_t, CImConn*> ConnMap_t;

// CImConn对象与用户id绑定
typedef std::unordered_map<uint32_t, CImConn*> UserMap_t;

// CImConn对象根据不同消息类型 分发事件处理
void imconn_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam);

#endif // _IMCONN_H_
