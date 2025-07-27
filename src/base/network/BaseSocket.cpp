/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: BaseSocket.cpp
 Update Time: Wed 14 Jun 2023 11:34:45 CST
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

#include "BaseSocket.h"
#include "EventDispatch.h"
using namespace std;

typedef hash_map<net_handle_t, CBaseSocket*> SocketMap;
SocketMap g_socket_map;

//向全局的 g_socket_map 中添加 CBaseSocket 对象
void AddBaseSocket(CBaseSocket* pSocket) {
	//其中 (net_handle_t)pSocket->GetSocket() 将 pSocket 的套接字句柄转换为 net_handle_t 类型作为键
	//pSocket 作为对应的值 插入到 g_socket_map 中
	//可以通过套接字句柄快速查找到对应的 CBaseSocket 对象，方便管理和操作套接字对象
    g_socket_map.insert(make_pair((net_handle_t)pSocket->GetSocket(), pSocket));
}

//从全局的 g_socket_map 中移除指定的 CBaseSocket 对象
void RemoveBaseSocket(CBaseSocket* pSocket) {
    g_socket_map.erase((net_handle_t)pSocket->GetSocket());
}

//从全局的 g_socket_map 中根据 net_handle_t 查找对应的 CBaseSocket 对象
CBaseSocket* FindBaseSocket(net_handle_t fd) {
    CBaseSocket* pSocket = NULL;
    SocketMap::iterator iter = g_socket_map.find(fd);
    if (iter != g_socket_map.end()) {
        pSocket = iter->second;
        pSocket->AddRef();
    }
    return pSocket;
}

//////////////////////////////////CBaseSocket//////////////////////////////////////////////
CBaseSocket::CBaseSocket() {
    // log("CBaseSocket::CBaseSocket\n");
    m_socket = INVALID_SOCKET;
    m_state = SOCKET_STATE_IDLE;
}

CBaseSocket::~CBaseSocket() {
    // log("CBaseSocket::~CBaseSocket, socket=%d\n", m_socket);
}

//////////////////////////////////CBaseSocket Listen Connect Send Recv Close/////////////////////////////////////
/**
 * 作为服务端进行 listen 操作
 * 网络层的核心部分操作
 * 该方法主要用于在服务器端创建一个监听套接字，绑定本地IP地址和端口，并开始监听连接请求
 * 监听套接字在有新的连接请求时会触发相应的事件回调函数
*/
/// @brief CBaseSocket类的Listen方法 用于将套接字绑定到指定的本地IP地址和端口，并开始监听连接请求
/// @param server_ip 要绑定的本地 IP 地址
/// @param port 要绑定的本地端口
/// @param callback 连接事件回调函数
/// @param callback_data 连接事件回调函数的用户数据
/// @return NETLIB_OK
int CBaseSocket::Listen(const char* server_ip, uint16_t port, callback_t callback, void* callback_data) {
    m_local_ip = server_ip;
    m_local_port = port;
    m_callback = callback;//连接事件回调函数
    m_callback_data = callback_data;//连接事件回调函数的用户数据

	// 1.调用 socket 函数创建一个TCP套接字，并将返回的套接字句柄存储到成员变量 m_socket 中
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == INVALID_SOCKET) {
        log_error("socket failed, err_code=%d, server_ip=%s, port=%u", _GetErrorCode(), server_ip, port);
        return NETLIB_ERROR;
    }

	// 2.设置套接字的地址重用和非阻塞属性
    _SetReuseAddr(m_socket);
    _SetNonblock(m_socket);

	// 3.结构体设置套接字的本地地址信息
    sockaddr_in serv_addr;
    _SetAddr(server_ip, port, &serv_addr);

	// 4.调用bind函数将套接字与本地地址绑定
    int ret = ::bind(m_socket, (sockaddr*)&serv_addr, sizeof(serv_addr));
    if (ret == SOCKET_ERROR) {
        log_error("bind failed, err_code=%d, server_ip=%s, port=%u", _GetErrorCode(), server_ip, port);
        // log_error("bind failed, server_ip=%s, port=%u, err_code=%d, err_msg=%s", server_ip, port, errno, strerror(errno));
        closesocket(m_socket);
        return NETLIB_ERROR;
    }

	// 5.调用listen函数开始监听连接请求 并更新套接字的状态为 SOCKET_STATE_LISTENING
    ret = listen(m_socket, 64);
    if (ret == SOCKET_ERROR) {
        log_error("listen failed, err_code=%d, server_ip=%s, port=%u", _GetErrorCode(), server_ip, port);
        closesocket(m_socket);
        return NETLIB_ERROR;
    }
    m_state = SOCKET_STATE_LISTENING;
    log_debug("CBaseSocket::Listen on %s:%d", server_ip, port);

	// 6.调用AddBaseSocket将当前对象添加到全局的套接字映射表中
    AddBaseSocket(this);

	// 7.调用 CEventDispatch::Instance()->AddEvent 将套接字句柄 和 待监听的事件类型注册到事件分发器中
    CEventDispatch::Instance()->AddEvent(m_socket, SOCKET_READ | SOCKET_EXCEP);
    return NETLIB_OK;
}


/**
 * 作为客户端进行 connect 操作
 * 方法主要用于在客户端与指定的服务器建立连接
 * 它会创建一个套接字并尝试连接到服务器，连接成功后会触发相应的事件回调函数。
 * 函数返回的套接字句柄可用于后续的读写操作或关闭连接。
*/
/// @brief CBaseSocket类的Connect 方法 用于与指定的服务器建立连接
/// @param server_ip 要连接的服务器的 IP 地址
/// @param port 要连接的服务器的端口
/// @param callback 连接事件回调函数
/// @param callback_data 连接事件回调函数的用户数据
/// @return 
net_handle_t CBaseSocket::Connect(const char* server_ip, uint16_t port, callback_t callback, void* callback_data) {
    log_debug("CBaseSocket::Connect, server_ip=%s, port=%d", server_ip, port);
    m_remote_ip = server_ip;
    m_remote_port = port;
    m_callback = callback;//连接事件回调函数
    m_callback_data = callback_data;//连接事件回调函数的用户数据

	// 1.调用socket函数创建一个 TCP 套接字，并将返回的套接字句柄存储到成员变量 m_socket 中。
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == INVALID_SOCKET) {
        log_error("socket failed, err_code=%d, server_ip=%s, port=%u", _GetErrorCode(), server_ip, port);
        return NETLIB_INVALID_HANDLE;
    }

	// 2.调用_SetNonblock和_SetNoDelay分别设置套接字的非阻塞和禁用Nagle算法属性
    _SetNonblock(m_socket);
    _SetNoDelay(m_socket);

	// 3.使用 sockaddr_in 结构体设置套接字的服务器地址信息
    sockaddr_in serv_addr;
    _SetAddr(server_ip, port, &serv_addr);

	// 4.调用 connect 函数与服务器建立连接 更新套接字的状态为 SOCKET_STATE_CONNECTING
    int ret = connect(m_socket, (sockaddr*)&serv_addr, sizeof(serv_addr));
    if ((ret == SOCKET_ERROR) && (!_IsBlock(_GetErrorCode()))) {
		//连接失败且错误码不表示阻塞状态 关闭套接字并返回 NETLIB_INVALID_HANDLE
        log_error("connect failed, err_code=%d, server_ip=%s, port=%u", _GetErrorCode(), server_ip, port);
        closesocket(m_socket);
        return NETLIB_INVALID_HANDLE;
    }
    m_state = SOCKET_STATE_CONNECTING;

	// 5.调用 AddBaseSocket 将当前对象添加到全局的套接字映射表中
    AddBaseSocket(this);

	// 6.调用 CEventDispatch::Instance()->AddEvent 将套接字句柄和待监听的事件类型注册到事件分发器中
    CEventDispatch::Instance()->AddEvent(m_socket, SOCKET_ALL);

	// 7.返回套接字句柄 (net_handle_t)m_socket
    return (net_handle_t)m_socket;
}

/**
 * 该方法用于向已连接的套接字发送数据，返回实际发送的字节数
 * 在发送数据时，如果发送缓冲区已满而无法立即发送，函数会返回阻塞状态，并将套接字注册到写事件中，
 * 等待下次可写事件触发再继续发送
*/
/// @brief CBaseSocket类的Send方法 用于向已连接的套接字发送数据
/// @param buf 指向要发送的数据缓冲区的指针
/// @param len 要发送的数据长度
/// @return 
int CBaseSocket::Send(void* buf, int len) {
	// 1.检查套接字的状态，如果不是已连接状态 (SOCKET_STATE_CONNECTED)，则返回 NETLIB_ERROR 表示发送失败
    if (m_state != SOCKET_STATE_CONNECTED) return NETLIB_ERROR;

	// 2.调用 send 函数发送数据
    int ret = send(m_socket, (char*)buf, len, 0);
    if (ret == SOCKET_ERROR) {
        int err_code = _GetErrorCode();
        if (_IsBlock(err_code)) {
			//数据发送失败 且错误码为阻塞状态 表示发送缓冲区已满
			/**
			 * 如果是WIN32或者苹果操作系统
			 * 此时会调用 CEventDispatch::Instance()->AddEvent(m_socket, SOCKET_WRITE) 将套接字注册到写事件中 等待下次可写事件触发再继续发送数据
			 * 如果是Linux操作系统 直接忽略改行代码
			 * 因为在Linux下，写事件是默认开启的，不需要手动注册到写事件中 发送缓冲区满时会自动触发写事件
			 * 通过注册写事件，可以实现非阻塞的方式发送数据，而不需要等待发送缓冲区可用。这样可以提高发送数据的效率
			*/
			#if ((defined _WIN32) || (defined __APPLE__))
			CEventDispatch::Instance()->AddEvent(m_socket, SOCKET_WRITE);
			#endif
            ret = 0;
            // log("socket send block fd=%d", m_socket);
        } else {
			//数据发送失败 且错误码不是阻塞状态 则记录错误日志
            log_error("send failed, err_code=%d, len=%d", err_code, len);
        }
    }

	// 3.返回发送的字节数 ret
    return ret;
}

/// @brief 用于接收数据 该方法调用操作系统提供的recv函数来接收数据并存储到指定的缓冲区中
/// @param buf 接收数据的缓冲区指针
/// @param len 期望接收的数据长度
/// @return 
int CBaseSocket::Recv(void* buf, int len) {
	/**
	 * m_socket 套接字描述符
	 * (char*)buf 接收数据的缓冲区指针
	 * len 期望接收的数据长度
	 * 标志参数 0
	 * 函数返回值为实际接收到的数据长度 返回值为-1表示接收出错
	*/
    return recv(m_socket, (char*)buf, len, 0);
}

//关闭套接字连接
int CBaseSocket::Close() {
	//调用CEventDispatch::Instance()->RemoveEvent函数将套接字从事件监听中移除 取消对其的读写事件监听
    CEventDispatch::Instance()->RemoveEvent(m_socket, SOCKET_ALL);
	//调用RemoveBaseSocket函数将套接字从全局套接字映射表中移除
    RemoveBaseSocket(this);
	//调用closesocket函数关闭套接字连接
    closesocket(m_socket);
	//调用ReleaseRef函数释放对CBaseSocket对象的引用
    ReleaseRef();
    return 0;
}


//////////////////////////////////CBaseSocket回调函数//////////////////////////////////////////////
/**
 * 用于处理套接字可读事件
 * 根据套接字的当前状态进行不同的处理：
*/
void CBaseSocket::OnRead() {
    if (m_state == SOCKET_STATE_LISTENING) {
		// 1.如果套接字处于监听状态（SOCKET_STATE_LISTENING），则调用_AcceptNewSocket函数接受新的连接
        _AcceptNewSocket();
    } else {
		// 2.否则通过调用ioctlsocket函数获取套接字接收缓冲区中 可读的字节数
		//#define ioctlsocket ioctl
		u_long avail = 0;
        int ret = ioctlsocket(m_socket, FIONREAD, &avail);
        if ((SOCKET_ERROR == ret) || (avail == 0)) {
			// 2-1.如果返回值为SOCKET_ERROR或avail为0，表示发生错误或没有可读数据
			// 通过调用回调函数m_callback将关闭事件（NETLIB_MSG_CLOSE）通知给上层处理
            m_callback(m_callback_data, NETLIB_MSG_CLOSE, (net_handle_t)m_socket, NULL);
        } else {
			// 有可读数据，通过调用回调函数m_callback将读事件（NETLIB_MSG_READ）通知给上层处理。
            m_callback(m_callback_data, NETLIB_MSG_READ, (net_handle_t)m_socket, NULL);
        }
    }
}

/**
 * 用于处理套接字可写事件
 * 根据套接字的当前状态进行不同的处理：
*/
void CBaseSocket::OnWrite() {

#if ((defined _WIN32) || (defined __APPLE__))
	//如果在Windows或苹果平台下，先调用CEventDispatch::Instance()->RemoveEvent函数移除套接字的 可写事件监听
    CEventDispatch::Instance()->RemoveEvent(m_socket, SOCKET_WRITE);
#endif

    if (m_state == SOCKET_STATE_CONNECTING) {
		// 1.如果套接字处于连接中状态（SOCKET_STATE_CONNECTING），则调用getsockopt函数获取套接字的错误状态
        int error = 0;
        socklen_t len = sizeof(error);
		
#ifdef _WIN32
        getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
#else
        getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (void*)&error, &len);
#endif

        if (error) {
			// 1-1.如果error不为0表示连接发生错误，通过调用回调函数m_callback将关闭事件（NETLIB_MSG_CLOSE）通知给上层处理
            m_callback(m_callback_data, NETLIB_MSG_CLOSE, (net_handle_t)m_socket, NULL);
        } else {
			// 1-2.如果error为0 连接成功，将套接字状态设置为已连接
            m_state = SOCKET_STATE_CONNECTED;
			// 并通过调用回调函数m_callback将确认事件（NETLIB_MSG_CONFIRM）通知给上层处理
            m_callback(m_callback_data, NETLIB_MSG_CONFIRM, (net_handle_t)m_socket, NULL);
        }
    } else {
		// 2.如果套接字不处于连接中状态，直接通过调用回调函数m_callback将写事件（NETLIB_MSG_WRITE）通知给上层处理
        m_callback(m_callback_data, NETLIB_MSG_WRITE, (net_handle_t)m_socket, NULL);
    }
}


//用于处理套接字关闭事件
void CBaseSocket::OnClose() {
	//将套接字的状态设置为正在关闭（SOCKET_STATE_CLOSING）
    m_state = SOCKET_STATE_CLOSING;
	//然后通过调用回调函数m_callback将关闭事件（NETLIB_MSG_CLOSE）通知给上层处理
    m_callback(m_callback_data, NETLIB_MSG_CLOSE, (net_handle_t)m_socket, NULL);
}


//////////////////////////////////CBaseSocket设置私有变量//////////////////////////////////////////////


/**
 * 设置套接字发送缓冲区的大小
 * 
 * 在网络通信中，发送缓冲区用于存储待发送的数据，套接字通过将数据写入发送缓冲区来进行发送操作。
 * 设置发送缓冲区的大小可以影响发送数据的效率和性能，
 * 	- 较大的发送缓冲区可以一次性发送更多的数据，减少系统调用的次数，提高发送速度
 *  - 过大的发送缓冲区可能会占用过多的内存资源
*/
void CBaseSocket::SetSendBufSize(uint32_t send_size) {
	/**
	 * 通过调用setsockopt函数设置 SO_SNDBUF 选项来指定发送缓冲区的大小
	 * m_socket 当前套接字的文件描述符
	 * SOL_SOCKET 套接字选项级别
	 * SO_SNDBUF 发送缓冲区大小的选项名
	 * &send_size 设置发送缓冲区大小的值
	 * 4 发送缓冲区大小的值的字节数
	*/
    int ret = setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, &send_size, 4);
	//如果设置套接字选项失败，将输出一条错误日志 提示设置SO_SNDBUF失败
    if (ret == SOCKET_ERROR) log_error("set SO_SNDBUF failed for fd=%d", m_socket);

	int size = 0;
    socklen_t len = 4;
	/**
	 * 通过调用getsockopt函数获取 实际设置的发送缓冲区大小 存储在变量 size 中，并打印日志进行记录
	 * m_socket 当前套接字的文件描述符
	 * SOL_SOCKET 套接字选项级别
	 * SO_SNDBUF 发送缓冲区大小的选项名
	 * &size 接收发送缓冲区大小的值的指针
	 * &len 发送缓冲区大小的值的字节数
	*/
    getsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, &size, &len);
    log_debug("socket=%d send_buf_size=%d", m_socket, size);
}

/**
 * 设置套接字的接收缓冲区大小
*/
void CBaseSocket::SetRecvBufSize(uint32_t recv_size) {
	/**
	 * 通过调用setsockopt函数设置 SO_SNDBUF 选项来指定接收缓冲区的大小
	 * m_socket 当前套接字的文件描述符
	 * SOL_SOCKET 套接字选项级别
	 * SO_RCVBUF 接受缓冲区大小的选项名
	 * &send_size 设置发送缓冲区大小的值
	 * 4 发送缓冲区大小的值的字节数
	*/
    int ret = setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, &recv_size, 4);
	//如果设置套接字选项失败，将输出一条错误日志 提示设置SO_RCVBUF失败
    if (ret == SOCKET_ERROR) log_error("set SO_RCVBUF failed for fd=%d", m_socket);

	int size = 0;
    socklen_t len = 4;
	/**
	 * 通过调用getsockopt函数获取 实际设置的接收缓冲区大小存储在变量 size 中，并打印日志进行记录
	 * m_socket 当前套接字的文件描述符
	 * SOL_SOCKET 套接字选项级别
	 * SO_RCVBUF 接收缓冲区大小的选项名
	 * &size 接收发送缓冲区大小的值的指针
	 * &len 发送缓冲区大小的值的字节数
	*/
    getsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, &size, &len);
    log_debug("socket=%d recv_buf_size=%d", m_socket, size);
}


//////////////////////////////////CBaseSocket 私有函数//////////////////////////////////////////////

//获取套接字操作的错误码
int CBaseSocket::_GetErrorCode() {
#ifdef _WIN32
	//调用 WSAGetLastError() 函数获取最近一次套接字操作的错误码
    return WSAGetLastError();
#else
	//在非Windows平台直接返回 errno，它是一个全局变量记录最近一次系统调用发生的错误码
    return errno;
#endif
}

//判断套接字操作是否处于阻塞状态
bool CBaseSocket::_IsBlock(int error_code) {
#ifdef _WIN32
	//在Windows平台判断错误码是否等于 WSAEINPROGRESS 或 WSAEWOULDBLOCK
	//如果是则返回 true，表示套接字操作处于阻塞状态
    return ((error_code == WSAEINPROGRESS) || (error_code == WSAEWOULDBLOCK));
#else
	//在非Windows平台，判断错误码是否等于 EINPROGRESS 或 EWOULDBLOCK
	//如果是则返回 true，表示套接字操作处于阻塞状态
    return ((error_code == EINPROGRESS) || (error_code == EWOULDBLOCK));
#endif
}

//设置套接字为非阻塞模式
void CBaseSocket::_SetNonblock(SOCKET fd) {
#ifdef _WIN32
	//在Windows平台，将 u_long 类型的变量 nonblock 设置为 1，表示启用非阻塞模式
	//然后使用 ioctlsocket 函数将套接字 fd 设置为非阻塞模式，并将 nonblock 作为参数传递
    u_long nonblock = 1;
    int ret = ioctlsocket(fd, FIONBIO, &nonblock);
#else
	//在非Windows平台，首先使用fcntl函数获取套接字 fd 的当前属性
	//并通过按位或操作符 | 将 O_NONBLOCK 标志与当前属性进行组合，以设置非阻塞模式
	//然后再次使用 fcntl 函数将修改后的属性应用到套接字 fd 上
    int ret = fcntl(fd, F_SETFL, O_NONBLOCK | fcntl(fd, F_GETFL));
#endif
    if (ret == SOCKET_ERROR) log_error("_SetNonblock failed, err_code=%d, fd=%d", _GetErrorCode(), fd);
}

/**
 * 关于 setsockopt 函数参数的解释：
 * int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
 *  - sockfd 套接字文件描述符，表示要设置选项的套接字
 *  - level 选项的协议层级。常见的协议层级包括
 * 		SOL_SOCKET 用于设置套接字级别的选项
 * 		IPPROTO_TCP  用于设置TCP协议级别的选项
 * 		IPPROTO_IP  用于设置IP协议级别的选项
 * 		其他协议层级 如IPPROTO_IPV6等
 *  - optname 选项的名称或标识符，用于指定要设置的具体选项
 * 		SO_REUSEADDR  允许地址重用，通常在服务器程序中使用
 * 		TCP_NODELAY 禁用Nagle算法，用于实时传输或需要低延迟的应用
 * 		SO_SNDBUF 发送缓冲区大小
 * 		SO_RCVBUF 接收缓冲区大小
 * 		其他选项 具体取决于所使用的协议和操作系统
 *  - optval 指向存储选项值的缓冲区的指针
 *  - optlen 选项值的长度
*/


/**
 * 当创建一个套接字并绑定到一个地址时，操作系统会保留该地址一段时间，以确保任何延迟到达的数据都能正确发送到该地址
 * 这意味着在关闭套接字后，一段时间内无法立即重新使用相同的地址和端口
 * 为了允许快速地重用套接字地址，可以使用SO_REUSEADDR套接字选项
 * 设置SO_REUSEADDR选项后，即使套接字处于TIME_WAIT状态，也可以立即重新绑定到相同的地址和端口
 * 设置后可以更灵活地管理套接字地址的重用，提高套接字的可用性和性能
*/
void CBaseSocket::_SetReuseAddr(SOCKET fd) {
    int reuse = 1;//定义reuse变量并设置为1，表示启用SO_REUSEADDR选项
	/**
	 * fd 套接字文件描述符
	 * SOL_SOCKET 套接字选项级别
	 * reuse 
	*/
    int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));
	// 如果设置失败 则将错误码_GetErrorCode和套接字文件描述符fd记录到日志中，以供调试和错误处理
    if (ret == SOCKET_ERROR) log_error("_SetReuseAddr failed, err_code=%d, fd=%d", _GetErrorCode(), fd);
}

/**
 * 设置套接字的TCP_NODELAY选项，以控制是否启用Nagle算法
 * Nagle算法是一种优化TCP传输的算法，
 * 其原理是通过将较小的数据块组合成较大的数据包进行传输，以减少网络上的小数据包数量。
 * 	 - 这种算法在某些情况下可以提高网络传输的效率，特别是在带宽有限或延迟较高的网络环境下
 *   - 然而对于某些实时性要求较高的应用，如实时音视频传输或游戏，使用Nagle算法会引入一定的延迟，因为数据需要等待一定的时间才会被发送出去
 * 	 - 在这些情况下禁用Nagle算法可以减少延迟，提高实时性
*/
void CBaseSocket::_SetNoDelay(SOCKET fd) {
    int nodelay = 1;//设置TCP_NODELAY选项为1
	/**
	 * fd 套接字文件描述符
	 * IPPROTO_TCP 用于设置TCP协议级别的选项
	 * nodelay 
	*/
    int ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));
    if (ret == SOCKET_ERROR) log_error("_SetNoDelay failed, err_code=%d, fd=%d", _GetErrorCode(), fd);
}

/// @brief 设置IPv4套接字地址
/// @param ip ip地址
/// @param port 端口号
/// @param pAddr sockaddr_in结构体
void CBaseSocket::_SetAddr(const char* ip, const uint16_t port, sockaddr_in* pAddr) {
	//1.确保没有任何旧数据残留
    memset(pAddr, 0, sizeof(sockaddr_in));
	//2.设置地址族为IPv4
    pAddr->sin_family = AF_INET;
	//3.将端口号转换为网络字节序（大端序）并赋值给sin_port成员变量
    pAddr->sin_port = htons(port);

	//4.将提供的IP地址转换为 网络字节序的32位无符号整数 并赋值给s_addr成员变量
    pAddr->sin_addr.s_addr = inet_addr(ip);
    if (pAddr->sin_addr.s_addr == INADDR_NONE) {
		//如果转换失败，即inet_addr返回INADDR_NONE
		//调用gethostbyname函数获取主机信息，通过提供的主机名或IP地址获取对应的 主机信息结构体hostent
        hostent* host = gethostbyname(ip);
        if (host == NULL) {
            log_error("gethostbyname failed, ip=%s, port=%u", ip, port);
            return;
        }
		//从主机信息结构体中获取主机的IP地址，并赋值给s_addr成员变量
        pAddr->sin_addr.s_addr = *(uint32_t*)host->h_addr;
    }

}


/**
 * 客户端发送请求连接到服务端
 * 服务端接受新的连接请求并创建一个新的套接字对象用于处理该连接
*/
void CBaseSocket::_AcceptNewSocket() {
    SOCKET fd = 0;//套接字描述符变量，用于接受新的连接
    sockaddr_in peer_addr;//用于保存对端地址信息的sockaddr_in结构体
    socklen_t addr_len = sizeof(sockaddr_in);//结构体的长度
    char ip_str[64];//保存对端IP地址的字符串

	//定义一个while循环
	//调用accept函数接受新的连接请求 并将返回的套接字描述符赋值给fd
	//如果接受成功（fd不等于INVALID_SOCKET）则进入循环体
    while ((fd = accept(m_socket, (sockaddr*)&peer_addr, &addr_len)) != INVALID_SOCKET) {
		// 1.创建一个新的CBaseSocket对象，用于处理新的连接
        CBaseSocket* pSocket = new CBaseSocket();
		// 2.将对端IP地址和端口号转换为字符串形式，并保存在 ip_str 中
        uint32_t ip = ntohl(peer_addr.sin_addr.s_addr);
        uint16_t port = ntohs(peer_addr.sin_port);
        snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", ip >> 24, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);

        log_debug("AcceptNewSocket, socket=%d from %s:%d\n", fd, ip_str, port);

		// 3.设置新的套接字对象的相关属性，包括套接字描述符、回调函数、回调数据、连接状态、对端IP地址和端口号
        pSocket->SetSocket(fd);
        pSocket->SetCallback(m_callback);
        pSocket->SetCallbackData(m_callback_data);
        pSocket->SetState(SOCKET_STATE_CONNECTED);
        pSocket->SetRemoteIP(ip_str);
        pSocket->SetRemotePort(port);

		// 4.调用_SetNoDelay函数设置套接字的TCP_NODELAY选项，以禁用Nagle算法
        _SetNoDelay(fd);

		// 5.调用_SetNonblock函数设置套接字为非阻塞模式
        _SetNonblock(fd);

		// 6.将新的套接字对象添加到套接字管理器中
        AddBaseSocket(pSocket);

		// 7.使用事件调度器（CEventDispatch）为新的套接字对象添加读事件和异常事件
        CEventDispatch::Instance()->AddEvent(fd, SOCKET_READ | SOCKET_EXCEP);

		// 8.调用回调函数，通知上层逻辑有新的连接建立，将套接字描述符作为参数传递
        m_callback(m_callback_data, NETLIB_MSG_CONNECT, (net_handle_t)fd, NULL);
    }
}


