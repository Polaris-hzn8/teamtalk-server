/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: LoginConn.cpp
 Update Time: Thu 15 Jun 2023 00:45:21 CST
 brief:
*/

#include "LoginConn.h"
#include "IM.Login.pb.h"
#include "IM.Other.pb.h"
#include "IM.Server.pb.h"
#include "public_define.h"

using namespace IM::BaseDefine;

//用于管理客户端连接对象 根据网络句柄找到对应的连接对象
static ConnMap_t g_client_conn_map;

//用于管理消息服务器连接对象 根据网络句柄找到对应的连接对象
static ConnMap_t g_msg_serv_conn_map;

//并发在线总人数
static uint32_t g_total_online_user_cnt = 0;        

//存储消息服务器的信息 根据消息服务器的连接句柄 uint32_t 映射到消息服务器信息的指针 msg_serv_info_t*
map<uint32_t, msg_serv_info_t*> g_msg_serv_info;    

/**
 * @brief
 * 这段代码的作用是定期触发连接对象的定时事件处理
 * 通过遍历连接对象的容器，并调用每个连接对象的 OnTimer() 函数，可以实现定时处理登录连接的逻辑
 * 定时器回调函数，用于处理登录连接的定时任务
 * 
 * @param callback_data 回调数据
 * @param msg           消息类型，用于区分不同的消息
 * @param handle        句柄 表示连接的标识
 * @param pParam        参数 用于传递额外的数据
 */
void login_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
    // 1.获取当前时间 cur_time，通过 get_tick_count() 函数获取
    uint64_t cur_time = get_tick_count();

    // 2.遍历 g_client_conn_map 中的所有连接对象，并调用每个连接对象的 OnTimer() 函数，传入当前时间 cur_time
    for (ConnMap_t::iterator it = g_client_conn_map.begin(); it != g_client_conn_map.end();) {
        //在每次迭代之前，将当前迭代器 it 的值赋给另一个迭代器 it_old，以便在删除连接对象时不影响迭代过程
        ConnMap_t::iterator it_old = it;
        //将迭代器 it 向后移动到下一个元素，以便在下一次循环时处理下一个连接对象
        it++;
        //通过 it_old->second 获取连接对象的指针，并将其转换为 CLoginConn* 类型
        CLoginConn* pConn = (CLoginConn*)it_old->second;
        //调用连接对象的 OnTimer() 函数，传入当前时间 cur_time，以进行定时处理
        pConn->OnTimer(cur_time);
    }

    // 3.遍历 g_msg_serv_conn_map 中的所有连接对象，并调用每个连接对象的 OnTimer() 函数，传入当前时间 cur_time
    for (ConnMap_t::iterator it = g_msg_serv_conn_map.begin(); it != g_msg_serv_conn_map.end();) {
        ConnMap_t::iterator it_old = it;
        it++;
        CLoginConn* pConn = (CLoginConn*)it_old->second;
        pConn->OnTimer(cur_time);
    }
}


//初始化登录连接的函数，用于注册登录连接的定时器
void init_login_conn() {
    //调用 netlib_register_timer() 函数注册一个定时器
    //将定时器回调函数 login_conn_timer_callback 作为参数传入，表示定时器触发时会调用该回调函数
    //第二个参数为 NULL，表示不传递任何参数给回调函数
    //最后一个参数 1000 表示定时器的触发间隔，以毫秒为单位，这里设置为每隔1秒触发一次
    netlib_register_timer(login_conn_timer_callback, NULL, 1000);
}

CLoginConn::CLoginConn() {}

CLoginConn::~CLoginConn() {}


/**
 * 函数的作用是关闭连接，并进行相应的清理工作 根据连接类型执行不同的操作
*/
void CLoginConn::Close()
{
    // 首先判断连接句柄 m_handle 是否为有效句柄 NETLIB_INVALID_HANDLE
    if (m_handle != NETLIB_INVALID_HANDLE) {
        // 1.调用netlib_close()函数关闭连接
        netlib_close(m_handle);
        if (m_conn_type == LOGIN_CONN_TYPE_CLIENT) {
            // 2.连接类型是客户端连接，则从 g_client_conn_map 中移除该连接
            g_client_conn_map.erase(m_handle);
        } else {
            // 3.连接类型是消息服务器连接，从 g_msg_serv_conn_map 中移除该连接
            g_msg_serv_conn_map.erase(m_handle);

            // 4.查找 g_msg_serv_info 中是否存在该连接句柄的消息服务器信息
            map<uint32_t, msg_serv_info_t*>::iterator it = g_msg_serv_info.find(m_handle);
            if (it != g_msg_serv_info.end()) {
                // 4-1.获取对应的消息服务器信息对象指针 pMsgServInfo
                msg_serv_info_t* pMsgServInfo = it->second;
                // 4-2.从总在线用户数 g_total_online_user_cnt 中减去该消息服务器的当前连接数
                g_total_online_user_cnt -= pMsgServInfo->cur_conn_cnt;
                // 4-3.输出日志信息，表示从消息服务器断开连接
                log("onclose from MsgServer: %s:%u ", pMsgServInfo->hostname.c_str(), pMsgServInfo->port);
                // 4-4.删除消息服务器信息对象，并释放内存
                delete pMsgServInfo;
                // 4-5.从 g_msg_serv_info 中移除消息服务器信息
                g_msg_serv_info.erase(it);
            }
        }
    }
    ReleaseRef();
}

/**
 * 连接成功后的回调函数，根据连接类型设置相应的回调函数和回调数据
 * 
 * 可以在连接建立时进行一些必要的设置，
 * 包括将连接句柄、连接类型以及回调函数等信息保存，
 * 并将连接对象插入到相应的连接映射容器中，以便后续的事件处理和管理
*/
void CLoginConn::OnConnect2(net_handle_t handle, int conn_type) {
    m_handle = handle;
    m_conn_type = conn_type;
    //根据连接类型，选择使用 g_client_conn_map 或 g_msg_serv_conn_map 作为连接映射容器，将其地址赋值给 conn_map 指针
    ConnMap_t* conn_map = &g_msg_serv_conn_map;
    if (conn_type == LOGIN_CONN_TYPE_CLIENT) {
        conn_map = &g_client_conn_map;
    } else {
        conn_map->insert(make_pair(handle, this));
    }
    //使用 netlib_option() 函数设置连接句柄的回调函数为 imconn_callback，以便在收到事件时回调相应的处理函数
    netlib_option(handle, NETLIB_OPT_SET_CALLBACK, (void*)imconn_callback);
    //使用 netlib_option() 函数设置连接句柄的回调数据为连接映射容器 conn_map 的地址，以便在回调函数中可以获取到相应的连接对象
    netlib_option(handle, NETLIB_OPT_SET_CALLBACK_DATA, (void*)conn_map);
}


/**
 * 连接关闭的回调函数，关闭连接
*/
void CLoginConn::OnClose()
{
    Close();
}


/**
 * 定时器回调函数，
 * 根据连接类型 在定时器触发时进行处理
*/
void CLoginConn::OnTimer(uint64_t curr_tick)
{
    //根据连接类型 在定时器触发时进行处理
    if (m_conn_type == LOGIN_CONN_TYPE_CLIENT) {
        // 1.如果是客户端连接 则执行客户端超时检查逻辑
        // 判断当前时间戳 curr_tick 是否超过上次接收数据的时间戳加上客户端超时时间 CLIENT_TIMEOUT
        // 如果超时，则调用Close()函数关闭连接
        if (curr_tick > m_last_recv_tick + CLIENT_TIMEOUT) Close();
    } else {
        // 2.否则执行消息服务器超时检查逻辑
        // 2-1.对于消息服务器连接，判断当前时间戳curr_tick是否超过上次发送心跳的时间戳加上心跳间隔时间 SERVER_HEARTBEAT_INTERVAL
        if (curr_tick > m_last_send_tick + SERVER_HEARTBEAT_INTERVAL) {
            //如果超时则构造心跳消息IMHeartBeat，封装成CImPdu对象并设置服务ID和命令ID
            IM::Other::IMHeartBeat msg;
            CImPdu pdu;
            pdu.SetPBMsg(&msg);
            pdu.SetServiceId(SID_OTHER);
            pdu.SetCommandId(CID_OTHER_HEARTBEAT);
            //调用SendPdu()函数发送心跳消息
            SendPdu(&pdu);
        }
        // 2-2.再次判断当前时间戳 curr_tick 是否超过上次接收数据的时间戳加上消息服务器超时时间 SERVER_TIMEOUT
        if (curr_tick > m_last_recv_tick + SERVER_TIMEOUT) {
            //如果超时，则输出日志信息并调用Close()函数关闭连接
            log("connection to MsgServer timeout ");
            Close();
        }
    }
}


/**
 * HandlePdu在imconn.cpp的OnRead函数中调用
 * 处理接收到的 PDU 消息的函数，根据命令 ID 分发处理不同的消息
*/
void CLoginConn::HandlePdu(CImPdu* pPdu)
{
    log("HandlePdu = %u", pPdu->GetCommandId());
    switch (pPdu->GetCommandId()) {
    case CID_OTHER_HEARTBEAT:
        break;
    case CID_OTHER_MSG_SERV_INFO:
        _HandleMsgServInfo(pPdu);
        break;
    case CID_OTHER_USER_CNT_UPDATE:
        _HandleUserCntUpdate(pPdu);
        break;
    case CID_LOGIN_REQ_MSGSERVER:
        _HandleMsgServRequest(pPdu);
        break;
    default:
        log("wrong msg, cmd id=%d ", pPdu->GetCommandId());
        break;
    }
}

/**
 * 处理消息服务器信息的函数，解析并存储消息服务器信息
*/
void CLoginConn::_HandleMsgServInfo(CImPdu* pPdu)
{
    msg_serv_info_t* pMsgServInfo = new msg_serv_info_t;
    IM::Server::IMMsgServInfo msg;
    msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength());

    pMsgServInfo->ip_addr1 = msg.ip1();
    pMsgServInfo->ip_addr2 = msg.ip2();
    pMsgServInfo->port = msg.port();
    pMsgServInfo->max_conn_cnt = msg.max_conn_cnt();
    pMsgServInfo->cur_conn_cnt = msg.cur_conn_cnt();
    pMsgServInfo->hostname = msg.host_name();
    g_msg_serv_info.insert(make_pair(m_handle, pMsgServInfo));

    g_total_online_user_cnt += pMsgServInfo->cur_conn_cnt;

    log("MsgServInfo, ip_addr1=%s, ip_addr2=%s, port=%d, max_conn_cnt=%d, cur_conn_cnt=%d, "
        "hostname: %s. ",
        pMsgServInfo->ip_addr1.c_str(), pMsgServInfo->ip_addr2.c_str(), pMsgServInfo->port, pMsgServInfo->max_conn_cnt,
        pMsgServInfo->cur_conn_cnt, pMsgServInfo->hostname.c_str());
}

/**
 * 处理用户在线人数更新的函数 根据用户上线和下线更新用户计数
*/
void CLoginConn::_HandleUserCntUpdate(CImPdu* pPdu)
{
    map<uint32_t, msg_serv_info_t*>::iterator it = g_msg_serv_info.find(m_handle);
    if (it != g_msg_serv_info.end()) {
        msg_serv_info_t* pMsgServInfo = it->second;
        IM::Server::IMUserCntUpdate msg;
        msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength());

        uint32_t action = msg.user_action();
        if (action == USER_CNT_INC) { // msg_server收到client登录后通知login_server上线加一
            pMsgServInfo->cur_conn_cnt++;
            g_total_online_user_cnt++;
        } else { // 下线减一
            pMsgServInfo->cur_conn_cnt--;
            g_total_online_user_cnt--;
        }

        log("%s:%d, cur_cnt=%u, total_cnt=%u ", pMsgServInfo->hostname.c_str(),
            pMsgServInfo->port, pMsgServInfo->cur_conn_cnt, g_total_online_user_cnt);
    }
}

/**
 * 处理消息服务器请求 根据在线用户数和消息服务器连接状态返回合适的消息服务器信息
*/
void CLoginConn::_HandleMsgServRequest(CImPdu* pPdu)
{
    IM::Login::IMMsgServReq msg;
    msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength());

    log("HandleMsgServReq. ");

    // no MessageServer available
    if (g_msg_serv_info.size() == 0) {
        IM::Login::IMMsgServRsp msg;
        msg.set_result_code(::IM::BaseDefine::REFUSE_REASON_NO_MSG_SERVER);
        CImPdu pdu;
        pdu.SetPBMsg(&msg);
        pdu.SetServiceId(SID_LOGIN);
        pdu.SetCommandId(CID_LOGIN_RES_MSGSERVER);
        pdu.SetSeqNum(pPdu->GetSeqNum());
        SendPdu(&pdu);
        Close();
        return;
    }

    // return a message server with minimum concurrent connection count
    msg_serv_info_t* pMsgServInfo;
    uint32_t min_user_cnt = (uint32_t)-1;
    map<uint32_t, msg_serv_info_t*>::iterator it_min_conn = g_msg_serv_info.end(), it;

    for (it = g_msg_serv_info.begin(); it != g_msg_serv_info.end(); it++) {
        pMsgServInfo = it->second;
        if ((pMsgServInfo->cur_conn_cnt < pMsgServInfo->max_conn_cnt) && (pMsgServInfo->cur_conn_cnt < min_user_cnt)) {
            it_min_conn = it;
            min_user_cnt = pMsgServInfo->cur_conn_cnt;
        }
    }

    if (it_min_conn == g_msg_serv_info.end()) {
        log("All TCP MsgServer are full ");
        IM::Login::IMMsgServRsp msg;
        msg.set_result_code(::IM::BaseDefine::REFUSE_REASON_MSG_SERVER_FULL);
        CImPdu pdu;
        pdu.SetPBMsg(&msg);
        pdu.SetServiceId(SID_LOGIN);
        pdu.SetCommandId(CID_LOGIN_RES_MSGSERVER);
        pdu.SetSeqNum(pPdu->GetSeqNum());
        SendPdu(&pdu);
    } else {
        IM::Login::IMMsgServRsp msg;
        msg.set_result_code(::IM::BaseDefine::REFUSE_REASON_NONE);
        msg.set_prior_ip(it_min_conn->second->ip_addr1);
        msg.set_backip_ip(it_min_conn->second->ip_addr2);
        msg.set_port(it_min_conn->second->port);
        CImPdu pdu;
        pdu.SetPBMsg(&msg);
        pdu.SetServiceId(SID_LOGIN);
        pdu.SetCommandId(CID_LOGIN_RES_MSGSERVER);
        pdu.SetSeqNum(pPdu->GetSeqNum());
        SendPdu(&pdu);
    }

    Close(); // after send MsgServResponse, active close the connection
}


