/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: netlib.cpp
 Update Time: Wed 14 Jun 2023 15:31:48 CST
 brief: 
    主要用于处理tcp连接，实现了一个网络库
    对外提供了调用的api ，它封装了CEventDispatch
*/

#include "netlib.h"
#include "BaseSocket.h"
#include "EventDispatch.h"

int netlib_init() {
    int ret = NETLIB_OK;
#ifdef _WIN32
    //在Windows平台下，它调用了WSAStartup()函数来初始化 Winsock 库
    //在其他平台下，该函数不需要进行额外的初始化操作
    WSADATA wsaData;
    WORD wReqest = MAKEWORD(1, 1);
    if (WSAStartup(wReqest, &wsaData) != 0) {
        ret = NETLIB_ERROR;
    }
#endif
    return ret;
}

int netlib_destroy() {
    int ret = NETLIB_OK;
#ifdef _WIN32
    //在Windows平台下，它调用了WSACleanup()函数来清理Winsock库的资源
    //在其他平台下，该函数不需要进行额外的清理操作
    if (WSACleanup() != 0) {
        ret = NETLIB_ERROR;
    }
#endif
    return ret;
}

//创建一个监听套接字并开始监听指定的 IP 地址和端口号
int netlib_listen(const char* server_ip, uint16_t port, callback_t callback, void* callback_data) {
    // 1.创建一个基础套接字对象CBaseSocket对象
    CBaseSocket* pSocket = new CBaseSocket();
    if (!pSocket) return NETLIB_ERROR;

    // 2.调用CBaseSocket对象的Listen()方法来进行实际的监听操作
    // 同时还会设置回调函数和回调数据，以便在有连接事件发生时进行处理
    int ret = pSocket->Listen(server_ip, port, callback, callback_data);
    if (ret == NETLIB_ERROR) delete pSocket;//释放先前创建的 CBaseSocket 对象

    return ret;
}

net_handle_t netlib_connect(const char* server_ip, uint16_t port, callback_t callback, void* callback_data) {
    // 1.创建一个基础套接字对象CBaseSocket对象
    CBaseSocket* pSocket = new CBaseSocket();
    if (!pSocket) return NETLIB_INVALID_HANDLE;

    // 2.调用CBaseSocket对象的Connect()方法来进行实际的连接操作
    // 同时还会设置回调函数和回调数据，以便在有连接事件发生时进行处理
    net_handle_t handle = pSocket->Connect(server_ip, port, callback, callback_data);
    if (handle == NETLIB_INVALID_HANDLE) delete pSocket;//释放先前创建的 CBaseSocket 对象

    // 3.返回一个有效的 net_handle_t 值表示连接句柄
    return handle;
}

/**
 * 补充问题：
 * netlib_connect与netlib_send中对分别使用delete pSocket和pSocket->ReleaseRef()操作有什么不同？
 * 
 * 1.在netlib_connect函数中，如果连接操作失败会通过 delete pSocket 删除创建的 CBaseSocket 对象
 * 这是因为在连接失败的情况下，创建的套接字对象没有被添加到套接字管理中，并且无法被其他地方引用和使用因此需要手动删除。
 * 
 * 2.而在netlib_send函数中，通过调用FindBaseSocket找到了对应的CBaseSocket 对象
 * 此时套接字对象仍然需要保持可用状态，以便在后续的发送操作中继续使用。
 * 因此通过调用 pSocket->ReleaseRef() 方法来释放之前增加的引用计数，而不是直接删除对象。
 * 
 * 3.总结：
 *  - delete pSocket 用于删除无法使用的套接字对象
 *  - pSocket->ReleaseRef() 用于释放引用计数，但保留套接字对象以供后续使用
*/


int netlib_send(net_handle_t handle, void* buf, int len) {
    // 1.根据handle套接字句柄查找相应的 CBaseSocket基础套接字对象
    CBaseSocket* pSocket = FindBaseSocket(handle);
    if (!pSocket) return NETLIB_ERROR;

    // 2.调用CBaseSocket对象的Send()方法来进行数据发送操作
    int ret = pSocket->Send(buf, len);

    // 3.调用ReleaseRef方法释放之前增加的引用计数，确保正确地管理套接字对象的生命周期
    pSocket->ReleaseRef();
    return ret;
}

int netlib_recv(net_handle_t handle, void* buf, int len) {
    // 1.根据handle套接字句柄查找相应的 CBaseSocket基础套接字对象
    CBaseSocket* pSocket = FindBaseSocket(handle);
    if (!pSocket) return NETLIB_ERROR;

    // 2.调用CBaseSocket对象的Recv()方法来进行数据接收操作
    int ret = pSocket->Recv(buf, len);

    // 3.调用ReleaseRef方法释放之前增加的引用计数，确保正确地管理套接字对象的生命周期
    pSocket->ReleaseRef();
    return ret;
}


int netlib_close(net_handle_t handle) {
    // 1.根据handle套接字句柄查找相应的 CBaseSocket基础套接字对象
    CBaseSocket* pSocket = FindBaseSocket(handle);
    if (!pSocket) return NETLIB_ERROR;

    // 2.调用CBaseSocket对象的Close()方法来关闭套接字
    // 这个方法会关闭套接字并释放与之相关的资源
    int ret = pSocket->Close();

    // 3.调用ReleaseRef方法释放之前增加的引用计数，确保正确地管理套接字对象的生命周期
    // 递减套接字对象的引用计数，并在引用计数为零时销毁套接字对象
    pSocket->ReleaseRef();
    return ret;
}



/// @brief 设置和获取套接字选项
/// @param handle 套接字句柄
/// @param opt 选项类型
/// @param optval 选项值
/// @return 
int netlib_option(net_handle_t handle, int opt, void* optval) {
    // 1.根据handle套接字句柄查找相应的 CBaseSocket基础套接字对象
    CBaseSocket* pSocket = FindBaseSocket(handle);
    if (!pSocket) return NETLIB_ERROR;

    // 2.检查选项类型 opt 是否合法，同时检查选项值 optval 是否为空
    if ((opt >= NETLIB_OPT_GET_REMOTE_IP) && !optval) return NETLIB_ERROR;

    // 3.根据选项类型 opt 执行相应的操作
    switch (opt) {
    case NETLIB_OPT_SET_CALLBACK:
        // 3-1.将回调函数设置为 optval
        pSocket->SetCallback((callback_t)optval);
        break;
    case NETLIB_OPT_SET_CALLBACK_DATA:
        // 3-2.将回调函数的数据设置为 optval
        pSocket->SetCallbackData(optval);
        break;
    case NETLIB_OPT_GET_REMOTE_IP:
        // 3-3.将远程 IP 地址存储到 optval 指向的字符串对象中
        *(string*)optval = pSocket->GetRemoteIP();
        break;
    case NETLIB_OPT_GET_REMOTE_PORT:
        // 3-4.将远程端口号存储到 optval 指向的 uint16_t 对象中
        *(uint16_t*)optval = pSocket->GetRemotePort();
        break;
    case NETLIB_OPT_GET_LOCAL_IP:
        // 3-5.将本地 IP 地址存储到 optval 指向的字符串对象中
        *(string*)optval = pSocket->GetLocalIP();
        break;
    case NETLIB_OPT_GET_LOCAL_PORT:
        // 3-6.将本地端口号存储到 optval 指向的 uint16_t 对象中
        *(uint16_t*)optval = pSocket->GetLocalPort();
        break;
    case NETLIB_OPT_SET_SEND_BUF_SIZE:
        // 3-7.将发送缓冲区大小设置为 optval 指向的 uint32_t 值
        pSocket->SetSendBufSize(*(uint32_t*)optval);
        break;
    case NETLIB_OPT_SET_RECV_BUF_SIZE:
        // 3-8.将接收缓冲区大小设置为 optval 指向的 uint32_t 值
        pSocket->SetRecvBufSize(*(uint32_t*)optval);
        break;
    }

    // 4.操作完成后调用pSocket->ReleaseRef()方法释放之前增加的引用计数
    pSocket->ReleaseRef();

    // 5.函数返回 NETLIB_OK 表示操作成功
    return NETLIB_OK;
}


/**
 * 通过提供合适的回调函数、用户数据和时间间隔，
 * 可以注册定时器并在指定的时间间隔内执行相应的操作
*/
/// @brief 注册定时器，以便在指定的时间间隔后触发回调函数
/// @param callback 函数指针指向一个函数，该函数在定时器触发时被调用
/// @param user_data 用户数据是一个指针，可以传递任意类型的数据给回调函数
/// @param interval 时间间隔
/// @return 
int netlib_register_timer(callback_t callback, void* user_data, uint64_t interval) {
    //调用 CEventDispatch::Instance() 获取 CEventDispatch 类的单例对象
    //调用单例对象的AddTimer方法，将回调函数callback、用户数据user_data 和时间间隔interval 添加到定时器列表中
    CEventDispatch::Instance()->AddTimer(callback, user_data, interval);
    return 0;
}

//删除定时器
int netlib_delete_timer(callback_t callback, void* user_data) {
    //调用 CEventDispatch::Instance() 获取 CEventDispatch 类的单例对象
    //调用单例对象的RemoveTimer方法，根据将回调函数callback和用户数据user_data删除定时器列表中的定时器
    CEventDispatch::Instance()->RemoveTimer(callback, user_data);
    return 0;
}

/**
 * 将指定的回调函数和用户数据添加到事件循环中
 * 在每次事件循环迭代时，都会调用注册的回调函数，并传递相应的用户数据
 * 这可用于周期性执行特定的任务 或处理其他需要在事件循环中执行的操作
*/
int netlib_add_loop(callback_t callback, void* user_data) {
    //调用 CEventDispatch::Instance() 获取 CEventDispatch 类的单例对象
    //调用单例对象的AddLoop方法，根据将回调函数callback和用户数据user_data添加到事件循环中
    CEventDispatch::Instance()->AddLoop(callback, user_data);
    return 0;
}

//启动网络库的事件循环 wait_timeout可以控制事件循环在等待事件时的超时时间
void netlib_eventloop(uint32_t wait_timeout) {
    //调用 CEventDispatch::Instance() 获取 CEventDispatch 类的单例对象
    //调用单例对象的StartDispatch方法 启动事件循环
    CEventDispatch::Instance()->StartDispatch(wait_timeout);
}

//停止网络库的事件循环
void netlib_stop_event() {
    //调用 CEventDispatch::Instance() 获取 CEventDispatch 类的单例对象
    //调用单例对象的StopDispatch方法 停止事件循环
    CEventDispatch::Instance()->StopDispatch();
}

//检查网络库的事件循环是否正在运行
bool netlib_is_running() {
    //调用 CEventDispatch::Instance() 获取 CEventDispatch 类的单例对象
    //调用单例对象的isRunning()方法 来获取事件循环的运行状态
    return CEventDispatch::Instance()->isRunning();
}


