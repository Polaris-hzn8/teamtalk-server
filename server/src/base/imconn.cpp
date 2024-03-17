/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: imconn.cpp
 Update Time: Mon 12 Jun 2023 23:48:11 CST
 brief:
*/

#include "imconn.h"

// static uint64_t g_send_pkt_cnt = 0;		// 发送数据包总数
// static uint64_t g_recv_pkt_cnt = 0;		// 接收数据包总数

//根据网路句柄 在ConnMap_t 类型的哈希映射容器中 查找对应的网络连接 CImConn*
static CImConn* FindImConn(ConnMap_t* imconn_map, net_handle_t handle) {
    CImConn* pConn = NULL;
    ConnMap_t::iterator iter = imconn_map->find(handle);
    if (iter != imconn_map->end()) {
        pConn = iter->second;
        pConn->AddRef();//增加对象的引用计数
    }
    return pConn;
}

/// @brief imconn_callback回调函数，用于处理与网络连接相关的消息 根据不同的消息类型分发事件处理
/// @param callback_data 回调函数的用户数据 指向 ConnMap_t 类型的哈希映射容器的指针
/// @param msg 接收到的消息类型的无符号字节
/// @param handle 网络句柄 用于标识一个网络连接
/// @param pParam 参数指针用于传递额外的参数
void imconn_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam) {
	// 1.忽略 handle 和 pParam 参数 函数体内部没有使用它们
    NOTUSED_ARG(handle);
    NOTUSED_ARG(pParam);

	//callback_data为空则直接返回
    if (!callback_data) return;

	// 2.将 callback_data 转换为 ConnMap_t* 类型的指针，并将其赋值给 conn_map
    ConnMap_t* conn_map = (ConnMap_t*)callback_data;

	// 3.使用FindImConn根据 handle网络句柄在 conn_map 中查找对应的 CImConn* 对象，并将结果赋值给 pConn
    CImConn* pConn = FindImConn(conn_map, handle);
    if (!pConn) return;

	// 4.根据接收到的msg类型执行相应的操作
    switch (msg) {
    case NETLIB_MSG_CONFIRM:
        pConn->OnConfirm();
        break;
    case NETLIB_MSG_READ:
        pConn->OnRead();
        break;
    case NETLIB_MSG_WRITE:
        pConn->OnWrite();
        break;
    case NETLIB_MSG_CLOSE:
        pConn->OnClose();
        break;
    default:
		//其他消息类型：记录错误日志
        log_error("!!!imconn_callback error msg: %d ", msg);
        break;
    }

	// 5.用 pConn 的 ReleaseRef 方法，释放对连接对象的引用
    pConn->ReleaseRef();
}

////////////////////////////////////////////////////CImConn////////////////////////////////////////////////////
CImConn::CImConn() {
    log_debug("CImConn::CImConn ");
    m_busy = false;//连接未忙碌
    m_handle = NETLIB_INVALID_HANDLE;//连接的句柄无效
    m_recv_bytes = 0;//接收的字节数为 0
    m_last_send_tick = m_last_recv_tick = get_tick_count();//设置为当前的系统时间戳
}

CImConn::~CImConn() {
    log_debug("CImConn::~CImConn, handle=%d ", m_handle);
}

/**
 * CImConn类中的Send方法 用于向连接发送数据
*/
int CImConn::Send(void* data, int len) {
	// 1.更新m_last_send_tick变量，记录最后一次发送数据的时间戳
    m_last_send_tick = get_tick_count();

	// 2.连接处于忙碌状态（之前的发送操作还未完成）那么将待发送的数据写入输出缓冲区m_out_buf，并返回数据长度len
    if (m_busy) {
        m_out_buf.Write(data, len);
        return len;
    }

	// 3.不处于忙碌状态 则进入循环进行实际的发送数据操作
    int offset = 0;
    int remain = len;
    while (remain > 0) {
		//将待发送数据分割为较小的块，并使用netlib_send函数发送数据块
        int send_size = remain;
        if (send_size > NETLIB_MAX_SOCKET_BUF_SIZE) send_size = NETLIB_MAX_SOCKET_BUF_SIZE;
        int ret = netlib_send(m_handle, (char*)data + offset, send_size);
		//如果发送失败（返回值ret小于等于0）终止循环
        if (ret <= 0) { ret = 0; break; }
		//每次发送后更新偏移量offset和剩余字节数remain
        offset += ret;
        remain -= ret;
    }

	// 4.如果仍有剩余数据未发送完毕（remain > 0）
    if (remain > 0) {
		// 将剩余数据写入输出缓冲区m_out_buf，并将m_busy标记为true，表示连接处于忙碌状态
        m_out_buf.Write((char*)data + offset, remain);
        m_busy = true;
        log_debug("send busy, remain=%d ", m_out_buf.GetWriteOffset());
    } else {
		// 5.如果所有数据都发送完毕 则调用OnWriteCompelete方法
        OnWriteCompelete();
    }

    return len;
}


//OnRead方法完成了从连接读取数据、解析CImPdu对象并处理的过程
void CImConn::OnRead() {
	// 1.通过循环不断读取数据
    for (;;) {
		// 1-1.在每次循环开始时，检查输入缓冲区m_in_buf的剩余可写空间free_buf_len
        uint32_t free_buf_len = m_in_buf.GetAllocSize() - m_in_buf.GetWriteOffset();
		// 如果不足READ_BUF_SIZE，则扩展输入缓冲区的大小为READ_BUF_SIZE
        if (free_buf_len < READ_BUF_SIZE) m_in_buf.Extend(READ_BUF_SIZE);
        log_debug("handle = %u, netlib_recv into, time = %u\n", m_handle, get_tick_count());

		// 1-2.调用netlib_recv函数从连接中接收数据
        int ret = netlib_recv(m_handle, m_in_buf.GetBuffer() + m_in_buf.GetWriteOffset(), READ_BUF_SIZE);
        if (ret <= 0) break;//ret小于等于0，表示接收失败或连接关闭，跳出循环

		// 1-3.更新已接收的字节数m_recv_bytes和最后一次接收数据的时间戳m_last_recv_tick
        m_recv_bytes += ret;
        m_in_buf.IncWriteOffset(ret);
        m_last_recv_tick = get_tick_count();
    }

	// 2.通过while循环从输入缓冲区中解析出CImPdu对象，并处理每个CImPdu对象
    CImPdu* pPdu = NULL;
    try {
		// 调用CImPdu::ReadPdu方法从输入缓冲区中读取一个完整的CImPdu对象pPdu
        while ((pPdu = CImPdu::ReadPdu(m_in_buf.GetBuffer(), m_in_buf.GetWriteOffset()))) {
			// 2-1.如果成功读取到了CImPdu对象，则获取该对象的长度pdu_len，并调用HandlePdu方法处理该对象
            uint32_t pdu_len = pPdu->GetLength();
            log_debug("handle = %u, pdu_len into = %u\n", m_handle, pdu_len);
            HandlePdu(pPdu);

			// 2-2.从输入缓冲区 m_in_buf 中读取指定长度的数据并将其丢弃
			// 通过将参数设置为 NULL 和 pdu_len，即表示从输入缓冲区中读取 pdu_len 长度的数据，但是并不保存读取的数据
			// 作用是丢弃输入缓冲区中指定长度的数据，以便在处理完相应的 CImPdu 对象后，将这部分数据从输入缓冲区中移除
            m_in_buf.Read(NULL, pdu_len);

			// 2-3.删除已处理的CImPdu对象，并将pPdu置为NULL
            delete pPdu;
            pPdu = NULL;
        }
    } catch (CPduException& ex) {
		// 2-4.如果在解析过程中捕获到CPduException异常，表示解析出现错误，记录错误信息并关闭连接
        log_error("!!!catch exception, sid=%u, cid=%u, err_code=%u, err_msg=%s, close the connection ",
            ex.GetServiceId(), ex.GetCommandId(), ex.GetErrorCode(), ex.GetErrorMsg());
        if (pPdu) {
            delete pPdu;
            pPdu = NULL;
        }
        OnClose();
    }
}

/**
 * OnWrite方法用于处理写入数据的事件
 * 将待发送的数据从输出缓冲区逐次发送出去，直到输出缓冲区中的数据全部发送完成
*/
void CImConn::OnWrite() {
    // 1.首先检查连接是否处于忙碌状态 如果连接不忙碌则直接返回，因为没有需要写入的数据
    if (!m_busy) return;

    // 2.如果连接处于忙碌状态，即存在待发送的数据 那么进入循环进行数据发送
    while (m_out_buf.GetWriteOffset() > 0) {
        // 2-1.获取待发送数据的长度，并根据网络库的最大缓冲区大小对发送长度进行调整
        int send_size = m_out_buf.GetWriteOffset();
        if (send_size > NETLIB_MAX_SOCKET_BUF_SIZE) send_size = NETLIB_MAX_SOCKET_BUF_SIZE;

        // 2-2.调用 netlib_send 函数将数据发送出去，并获取实际发送的字节数
        int ret = netlib_send(m_handle, m_out_buf.GetBuffer(), send_size);

        // 2-3.发送失败或返回的字节数小于等于0，表示发送遇到了错误循环中断
        if (ret <= 0) {
            ret = 0;
            break;
        }

        // 2-4.发送成功，从输出缓冲区m_out_buf中读取已发送的数据，并将其从缓冲区中丢弃
        m_out_buf.Read(NULL, ret);
    }

    // 3.检查输出缓冲区的写入偏移量是否为0 如果为0，表示所有待发送的数据已经发送完毕，将连接的忙碌状态设置为false
    if (m_out_buf.GetWriteOffset() == 0) m_busy = false;
    log_debug("onWrite, remain=%d ", m_out_buf.GetWriteOffset());
}
