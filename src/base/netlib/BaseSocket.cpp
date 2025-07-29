/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: BaseSocket.cpp
 Update Time: Wed 14 Jun 2023 11:34:45 CST
 brief:
    非阻塞套接字包装类/支持跨平台 CBaseSocket
	a wrap for non-block socket class for Windows, LINUX and MacOS X platform
	 1.封装了套接字的基本操作，使网络编程更方便
	 2.支持监听和接受连接请求，可以用于创建服务器端套接字
	 3.支持发起连接到远程服务器，可以用于创建客户端套接字
	 4.提供发送和接收数据的方法，可用于在网络上进行数据的传输
	 5.处理套接字的事件，例如可读事件、可写事件和关闭事件，通过回调函数的方式进行处理
*/

#include "BaseSocket.h"
#include "EventDispatch.h"
#include <unordered_map>
using namespace std;

typedef unordered_map<net_handle_t, CBaseSocket*> SocketMap;

SocketMap g_socket_map;

void AddBaseSocket(CBaseSocket* pSocket)
{
    g_socket_map.insert(make_pair((net_handle_t)pSocket->GetSocket(), pSocket));
}

void RemoveBaseSocket(CBaseSocket* pSocket)
{
    g_socket_map.erase((net_handle_t)pSocket->GetSocket());
}

CBaseSocket* FindBaseSocket(net_handle_t fd)
{
    CBaseSocket* pSocket = NULL;
    SocketMap::iterator iter = g_socket_map.find(fd);
    if (iter != g_socket_map.end()) {
        pSocket = iter->second;
        pSocket->AddRef();
    }
    return pSocket;
}

//////////////////////////////////CBaseSocket//////////////////////////////////////////////
CBaseSocket::CBaseSocket()
{
    // log("CBaseSocket::CBaseSocket\n");
    m_socket = INVALID_SOCKET;
    m_state = SOCKET_STATE_IDLE;
}

CBaseSocket::~CBaseSocket()
{
    // log("CBaseSocket::~CBaseSocket, socket=%d\n", m_socket);
}

/**
 * 设置套接字发送缓冲区的大小
 * 在网络通信中，发送缓冲区用于存储待发送的数据，套接字通过将数据写入发送缓冲区来进行发送操作。
 * 设置发送缓冲区的大小可以影响发送数据的效率和性能，
 * 	- 较大的发送缓冲区可以一次性发送更多的数据，减少系统调用的次数，提高发送速度
 *  - 过大的发送缓冲区可能会占用过多的内存资源
*/
void CBaseSocket::SetSendBufSize(uint32_t send_size)
{
    // 设置发送缓冲区大小
    int ret = setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, &send_size, 4);
    if (ret == SOCKET_ERROR)
        log_error("set SO_SNDBUF failed for fd=%d", m_socket);

	int size = 0;
    socklen_t len = 4;
    getsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, &size, &len);// 获取发送缓冲区大小

    log_debug("socket=%d send_buf_size=%d", m_socket, size);
}

void CBaseSocket::SetRecvBufSize(uint32_t recv_size)
{
    // 设置接收缓冲区大小
    int ret = setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, &recv_size, 4);
    if (ret == SOCKET_ERROR)
        log_error("set SO_RCVBUF failed for fd=%d", m_socket);

	int size = 0;
    socklen_t len = 4;
    getsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, &size, &len);// 获取接收缓冲区大小

    log_debug("socket=%d recv_buf_size=%d", m_socket, size);
}

/**
 * @brief 监听连接请求        并处理连接事件
 * @param server_ip         绑定的本地IP地址
 * @param port              绑定的本地端口
 * @param callback          连接事件回调函数
 * @param callback_data     连接事件回调函数参数
 * @return int 
 */
int CBaseSocket::Listen(const char* server_ip, uint16_t port, callback_t callback, void* callback_data)
{
    m_local_port = port;
    m_local_ip = server_ip;
    m_callback = callback;
    m_callback_data = callback_data;

    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == INVALID_SOCKET) {
        log_error("socket failed, err_code=%d, server_ip=%s, port=%u", _GetErrorCode(), server_ip, port);
        return NETLIB_ERROR;
    }

    _SetReuseAddr(m_socket);
    _SetNonblock(m_socket);

    sockaddr_in serv_addr;
    _SetAddr(server_ip, port, &serv_addr);

    int ret = ::bind(m_socket, (sockaddr*)&serv_addr, sizeof(serv_addr));
    if (ret == SOCKET_ERROR) {
        log_error("bind failed, err_code=%d, server_ip=%s, port=%u", _GetErrorCode(), server_ip, port);
        // log_error("bind failed, server_ip=%s, port=%u, err_code=%d, err_msg=%s",
        //     server_ip, port, errno, strerror(errno));
        closesocket(m_socket);
        return NETLIB_ERROR;
    }

    ret = listen(m_socket, 64);
    if (ret == SOCKET_ERROR) {
        log_error("listen failed, err_code=%d, server_ip=%s, port=%u", _GetErrorCode(), server_ip, port);
        closesocket(m_socket);
        return NETLIB_ERROR;
    }

    m_state = SOCKET_STATE_LISTENING;
    log_debug("CBaseSocket::Listen on %s:%d", server_ip, port);

    AddBaseSocket(this);    // 添加到套接字映射表中

    CEventDispatch::Instance()->AddEvent(m_socket, SOCKET_READ | SOCKET_EXCEP); // 监听套接字读事件

    return NETLIB_OK;
}

/**
 * @brief 作为客户端进行连接操作
 * 
 * @param server_ip         目标服务器IP地址   
 * @param port              目标服务器端口
 * @param callback          回调函数
 * @param callback_data     回调函数参数
 * @return net_handle_t 
 */
net_handle_t CBaseSocket::Connect(const char* server_ip, uint16_t port, callback_t callback, void* callback_data)
{
    log_debug("CBaseSocket::Connect, server_ip=%s, port=%d", server_ip, port);

    m_remote_port = port;
    m_remote_ip = server_ip;
    m_callback = callback;
    m_callback_data = callback_data;

    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == INVALID_SOCKET) {
        log_error("socket failed, err_code=%d, server_ip=%s, port=%u", _GetErrorCode(), server_ip, port);
        return NETLIB_INVALID_HANDLE;
    }

    _SetNonblock(m_socket);
    _SetNoDelay(m_socket);

    sockaddr_in serv_addr;
    _SetAddr(server_ip, port, &serv_addr);

    int ret = connect(m_socket, (sockaddr*)&serv_addr, sizeof(serv_addr));
    if ((ret == SOCKET_ERROR) && (!_IsBlock(_GetErrorCode()))) {
        log_error("connect failed, err_code=%d, server_ip=%s, port=%u", _GetErrorCode(), server_ip, port);
        closesocket(m_socket);
        return NETLIB_INVALID_HANDLE;
    }

    m_state = SOCKET_STATE_CONNECTING;

    AddBaseSocket(this);

    CEventDispatch::Instance()->AddEvent(m_socket, SOCKET_ALL);

    return (net_handle_t)m_socket;
}

int CBaseSocket::Send(void* buf, int len)
{
    if (m_state != SOCKET_STATE_CONNECTED)
        return NETLIB_ERROR;
	// 数据发送
    int ret = send(m_socket, (char*)buf, len, 0);
    if (ret == SOCKET_ERROR) {
        int err_code = _GetErrorCode();
        if (_IsBlock(err_code)) {
			/**
			 * 发送失败且错误码为阻塞状态 -> 发送缓冲区已满
             * 1.如果是WIN32或者苹果操作系统
			 * 此时调用 CEventDispatch::Instance()->AddEvent(m_socket, SOCKET_WRITE) 将套接字注册到写事件中 等待下次可写事件触发再继续发送数据
			 * 2.如果是Linux操作系统 直接忽略改行代码
			 * 因为在Linux下写事件是默认开启的，不需要手动注册到写事件中 发送缓冲区满时会自动触发写事件
			 * 通过注册写事件，可以实现非阻塞的方式发送数据，而不需要等待发送缓冲区可用。这样可以提高发送数据的效率
			*/
#if ((defined _WIN32) || (defined __APPLE__))
			CEventDispatch::Instance()->AddEvent(m_socket, SOCKET_WRITE);
#endif
            ret = 0;
            // log("socket send block fd=%d", m_socket);
        } else {
            log_error("send failed, err_code=%d, len=%d", err_code, len);
        }
    }
    return ret;
}

int CBaseSocket::Recv(void* buf, int len)
{
    return recv(m_socket, (char*)buf, len, 0);
}

int CBaseSocket::Close()
{
    CEventDispatch::Instance()->RemoveEvent(m_socket, SOCKET_ALL);
    RemoveBaseSocket(this);
    closesocket(m_socket);
    ReleaseRef();
    return 0;
}

void CBaseSocket::OnRead()
{
    if (m_state == SOCKET_STATE_LISTENING) {
        /* 调用_AcceptNewSocket 接受新连接 */
        _AcceptNewSocket();
    } else {
        /* 获取套接字接收缓冲区中 可读的字节数 */
		u_long avail = 0;
        int ret = ioctlsocket(m_socket, FIONREAD, &avail);
        if ((SOCKET_ERROR == ret) || (avail == 0)) {
            /* 发生错误或没有可读数据 */
            m_callback(m_callback_data, NETLIB_MSG_CLOSE, (net_handle_t)m_socket, NULL);
        } else {
            /* 正常有可读数据 */
            m_callback(m_callback_data, NETLIB_MSG_READ, (net_handle_t)m_socket, NULL);
        }
    }
}

void CBaseSocket::OnWrite()
{
#if ((defined _WIN32) || (defined __APPLE__))
    /* 移除套接字的可写事件监听 */
    CEventDispatch::Instance()->RemoveEvent(m_socket, SOCKET_WRITE);
#endif
    if (m_state == SOCKET_STATE_CONNECTING) {
        /* 套接字处于连接中状态 获取套接字的错误状态 */
        int error = 0;
        socklen_t len = sizeof(error);
#ifdef _WIN32
        getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
#else
        getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (void*)&error, &len);
#endif
        if (error) {
            /* 连接错误 */
            m_callback(m_callback_data, NETLIB_MSG_CLOSE, (net_handle_t)m_socket, NULL);
        } else {
            /* 连接成功 将套接字状态设置为已连接 */
            m_state = SOCKET_STATE_CONNECTED;
            m_callback(m_callback_data, NETLIB_MSG_CONFIRM, (net_handle_t)m_socket, NULL);
        }
    } else {
        /* 套接字不处于连接中状态 */
        m_callback(m_callback_data, NETLIB_MSG_WRITE, (net_handle_t)m_socket, NULL);
    }
}

void CBaseSocket::OnClose()
{
    m_state = SOCKET_STATE_CLOSING;
    m_callback(m_callback_data, NETLIB_MSG_CLOSE, (net_handle_t)m_socket, NULL);
}

////////////////////////////////// 私有函数//////////////////////////////////////////////
int CBaseSocket::_GetErrorCode()
{
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

bool CBaseSocket::_IsBlock(int error_code)
{
#ifdef _WIN32
    return ((error_code == WSAEINPROGRESS) || (error_code == WSAEWOULDBLOCK));
#else
    return ((error_code == EINPROGRESS) || (error_code == EWOULDBLOCK));
#endif
}

void CBaseSocket::_SetNonblock(SOCKET fd)
{
    int ret = SOCKET_ERROR;
#ifdef _WIN32
    u_long nonblock = 1;
    ret = ioctlsocket(fd, FIONBIO, &nonblock);
#else
    ret = fcntl(fd, F_SETFL, O_NONBLOCK | fcntl(fd, F_GETFL));
#endif
    if (ret == SOCKET_ERROR)
        log_error("_SetNonblock failed, err_code=%d, fd=%d", _GetErrorCode(), fd);
}

void CBaseSocket::_SetReuseAddr(SOCKET fd)
{
    int reuse = 1;
    int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));
    if (ret == SOCKET_ERROR)
        log_error("_SetReuseAddr failed, err_code=%d, fd=%d", _GetErrorCode(), fd);
}

/**
 * 设置套接字的TCP_NODELAY选项，以控制是否启用Nagle算法
 * Nagle算法是一种优化TCP传输的算法，
 * 其原理是通过将较小的数据块组合成较大的数据包进行传输，以减少网络上的小数据包数量。
 * 	 - 这种算法在某些情况下可以提高网络传输的效率，特别是在带宽有限或延迟较高的网络环境下
 *   - 然而对于某些实时性要求较高的应用，如实时音视频传输或游戏，使用Nagle算法会引入一定的延迟，因为数据需要等待一定的时间才会被发送出去
 * 	 - 在这些情况下禁用Nagle算法可以减少延迟，提高实时性
*/
void CBaseSocket::_SetNoDelay(SOCKET fd)
{
    int nodelay = 1;
    int ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));
    if (ret == SOCKET_ERROR)
        log_error("_SetNoDelay failed, err_code=%d, fd=%d", _GetErrorCode(), fd);
}

void CBaseSocket::_SetAddr(const char* ip, const uint16_t port, sockaddr_in* pAddr)
{
    memset(pAddr, 0, sizeof(sockaddr_in));
    pAddr->sin_family = AF_INET;
    pAddr->sin_port = htons(port);
    pAddr->sin_addr.s_addr = inet_addr(ip);
    if (pAddr->sin_addr.s_addr == INADDR_NONE) {
        /* 获取对应ip地址 */
        hostent* host = gethostbyname(ip);
        if (host == NULL) {
            log_error("gethostbyname failed, ip=%s, port=%u", ip, port);
            return;
        }
        pAddr->sin_addr.s_addr = *(uint32_t*)host->h_addr;
    }
}

void CBaseSocket::_AcceptNewSocket()
{
    SOCKET fd = 0;
    sockaddr_in peer_addr;
    socklen_t addr_len = sizeof(sockaddr_in);

    char ip_str[64] = { 0 };
    while ((fd = accept(m_socket, (sockaddr*)&peer_addr, &addr_len)) != INVALID_SOCKET) {
        CBaseSocket* pSocket = new CBaseSocket();
        uint32_t ip = ntohl(peer_addr.sin_addr.s_addr);
        uint16_t port = ntohs(peer_addr.sin_port);
        snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", ip >> 24, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);

        log_debug("AcceptNewSocket, socket=%d from %s:%d\n", fd, ip_str, port);

        pSocket->SetSocket(fd);
        pSocket->SetCallback(m_callback);
        pSocket->SetCallbackData(m_callback_data);
        pSocket->SetState(SOCKET_STATE_CONNECTED);
        pSocket->SetRemoteIP(ip_str);
        pSocket->SetRemotePort(port);

        _SetNoDelay(fd);    // 禁用Nagle算法
        _SetNonblock(fd);   // 非阻塞模式

        AddBaseSocket(pSocket);

        CEventDispatch::Instance()->AddEvent(fd, SOCKET_READ | SOCKET_EXCEP);

        // 通知上层逻辑有新的连接建立 并将套接字描述符作为参数传递
        m_callback(m_callback_data, NETLIB_MSG_CONNECT, (net_handle_t)fd, NULL);
    }
}

