/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: BaseSocket.h
 Update Time: Wed 14 Jun 2023 11:25:30 CST
 brief:
 	非阻塞套接字包装类/支持跨平台 CBaseSocket
	a wrap for non-block socket class for Windows, LINUX and MacOS X platform
	 1.封装了套接字的基本操作，使网络编程更方便
	 2.支持监听和接受连接请求，可以用于创建服务器端套接字
	 3.支持发起连接到远程服务器，可以用于创建客户端套接字
	 4.提供发送和接收数据的方法，可用于在网络上进行数据的传输
	 5.处理套接字的事件，例如可读事件、可写事件和关闭事件，通过回调函数的方式进行处理
*/

#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "common/util.h"
#include "common/ostype.h"

enum {
	SOCKET_STATE_IDLE,
	SOCKET_STATE_LISTENING,
	SOCKET_STATE_CONNECTING,
	SOCKET_STATE_CONNECTED,
	SOCKET_STATE_CLOSING
};

class CBaseSocket : public CRefObject
{
public:
	CBaseSocket();
	virtual ~CBaseSocket();

	SOCKET GetSocket() { return m_socket; }
	void SetSocket(SOCKET fd) { m_socket = fd; }
	void SetState(uint8_t state) { m_state = state; }

	void SetCallback(callback_t callback) { m_callback = callback; }
	void SetCallbackData(void* data) { m_callback_data = data; }
	void SetRemoteIP(char* ip) { m_remote_ip = ip; }
	void SetRemotePort(uint16_t port) { m_remote_port = port; }
	void SetSendBufSize(uint32_t send_size);
	void SetRecvBufSize(uint32_t recv_size);

	const char*	GetRemoteIP() { return m_remote_ip.c_str(); }
	uint16_t	GetRemotePort() { return m_remote_port; }
	const char*	GetLocalIP() { return m_local_ip.c_str(); }
	uint16_t	GetLocalPort() { return m_local_port; }

public:
	// 监听连接/服务器
	int Listen(
		const char*		server_ip, 
		uint16_t		port,
		callback_t		callback,
		void*			callback_data);
	
	// 发起连接/客户端
	net_handle_t Connect(
		const char*		server_ip, 
		uint16_t		port,
		callback_t		callback,
		void*			callback_data);

	int Send(void* buf, int len);
	int Recv(void* buf, int len);
	int Close();
	
	void OnRead();
	void OnWrite();
	void OnClose();

private:
	int _GetErrorCode();
	bool _IsBlock(int error_code);

	void _SetNonblock(SOCKET fd);
	void _SetReuseAddr(SOCKET fd);
	void _SetNoDelay(SOCKET fd);
	void _SetAddr(const char* ip, const uint16_t port, sockaddr_in* pAddr);
	
	void _AcceptNewSocket();
private:
	std::string		m_remote_ip;		//远程IP地址
	uint16_t		m_remote_port;		//远程端口号
	std::string		m_local_ip;			//本地IP地址
	uint16_t		m_local_port;		//本地端口号

	callback_t		m_callback;			//回调函数(用于处理套接字事件)
	void*			m_callback_data;	//回调函数参数

	uint8_t			m_state;			//套接字的状态
	SOCKET			m_socket;			//套接字描述符
};

CBaseSocket* FindBaseSocket(net_handle_t fd);

#endif
