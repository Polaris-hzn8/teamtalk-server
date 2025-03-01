/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: BaseSocket.h
 Update Time: Wed 14 Jun 2023 11:25:30 CST
 brief:
	基于非阻塞套接字的包装类 CBaseSocket 用于在Windows、Linux和MacOS X平台上进行网络通信
	a wrap for non-block socket class for Windows, LINUX and MacOS X platform
	 1.封装了套接字的基本操作，使网络编程更加方便和简化
	 2.支持监听和接受连接请求，可以用于创建服务器端套接字
	 3.支持发起连接到远程服务器，可以用于创建客户端套接字
	 4.提供了发送和接收数据的方法，用于在网络上进行数据的传输
	 5.处理套接字的事件，例如可读事件、可写事件和关闭事件，通过回调函数的方式进行处理
	 6.提供了一些辅助函数，如设置套接字的选项和获取错误代码等
*/

#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "ostype.h"
#include "util.h"

enum {
	SOCKET_STATE_IDLE,
	SOCKET_STATE_LISTENING,
	SOCKET_STATE_CONNECTING,
	SOCKET_STATE_CONNECTED,
	SOCKET_STATE_CLOSING
};

class CBaseSocket : public CRefObject {
public:
	CBaseSocket();
	virtual ~CBaseSocket();

	//获取套接字描述符
	SOCKET GetSocket() { return m_socket; }
	//设置套接字描述符
	void SetSocket(SOCKET fd) { m_socket = fd; }
	//设置套接字的状态
	void SetState(uint8_t state) { m_state = state; }

	//设置回调函数
	void SetCallback(callback_t callback) { m_callback = callback; }
	//设置回调函数的参数数据
	void SetCallbackData(void* data) { m_callback_data = data; }
	//设置远程IP地址
	void SetRemoteIP(char* ip) { m_remote_ip = ip; }
	//设置远程端口号
	void SetRemotePort(uint16_t port) { m_remote_port = port; }
	//设置发送缓冲区大小
	void SetSendBufSize(uint32_t send_size);
	//设置接收缓冲区大小
	void SetRecvBufSize(uint32_t recv_size);

	//获取远程IP地址
	const char*	GetRemoteIP() { return m_remote_ip.c_str(); }
	//获取远程端口号
	uint16_t	GetRemotePort() { return m_remote_port; }
	//获取本地IP地址
	const char*	GetLocalIP() { return m_local_ip.c_str(); }
	//获取本地端口号
	uint16_t	GetLocalPort() { return m_local_port; }

public:
	//监听指定IP地址和端口号的连接请求
	int Listen(
		const char*		server_ip, 
		uint16_t		port,
		callback_t		callback,
		void*			callback_data);

	//发起到指定IP地址和端口号的连接
	net_handle_t Connect(
		const char*		server_ip, 
		uint16_t		port,
		callback_t		callback,
		void*			callback_data);

	//发送数据
	int Send(void* buf, int len);
	//接收数据
	int Recv(void* buf, int len);
	//关闭套接字
	int Close();

public:
	//套接字可读事件的回调函数
	void OnRead();
	//套接字可写事件的回调函数
	void OnWrite();
	//套接字关闭事件的回调函数
	void OnClose();

private:
	//获取错误代码
	int _GetErrorCode();
	//判断错误代码是否表示阻塞状态
	bool _IsBlock(int error_code);

	//设置套接字为非阻塞模式
	void _SetNonblock(SOCKET fd);
	//设置套接字的地址复用选项
	void _SetReuseAddr(SOCKET fd);
	//设置套接字的TCP_NODELAY选项
	void _SetNoDelay(SOCKET fd);
	//设置套接字地址
	void _SetAddr(const char* ip, const uint16_t port, sockaddr_in* pAddr);

	//接受新的连接套接字
	void _AcceptNewSocket();
private:
	string			m_remote_ip;//远程IP地址
	uint16_t		m_remote_port;//远程端口号
	string			m_local_ip;//本地IP地址
	uint16_t		m_local_port;//本地端口号

	callback_t		m_callback;//回调函数指针，用于处理套接字事件
	void*			m_callback_data;//回调函数的参数数据

	uint8_t			m_state;//套接字的状态，可能的取值为 SOCKET_STATE_IDLE、等
	SOCKET			m_socket;//套接字描述符
};

//根据fd查找到对应的CBaseSocket对象，然后去处理对应的业务逻辑
CBaseSocket* FindBaseSocket(net_handle_t fd);

#endif
