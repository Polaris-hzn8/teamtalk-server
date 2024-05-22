/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: file_client_conn.cpp
 Update Time: Thu 15 Jun 2023 00:40:12 CST
 brief:
*/

#include "file_server/file_client_conn.h"

#include "base/pb/protocol/IM.File.pb.h"
#include "base/pb/protocol/IM.Other.pb.h"

#include "base/im_conn_util.h"

#include "file_server/config_util.h"
#include "file_server/transfer_task_manager.h"

using namespace IM::BaseDefine;

static ConnMap_t g_file_client_conn_map; // connection with others, on connect insert...

void FileClientConnCallback(void* callback_data, uint8_t msg, uint32_t handle, void* param)
{
    if (msg == NETLIB_MSG_CONNECT) {
        FileClientConn* conn = new FileClientConn();
        conn->OnConnect(handle);
    } else {
        log("!!!error msg: %d ", msg);
    }
}

void FileClientConnTimerCallback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
    uint64_t cur_time = get_tick_count();
    for (ConnMap_t::iterator it = g_file_client_conn_map.begin(); it != g_file_client_conn_map.end();) {
        ConnMap_t::iterator it_old = it;
        it++;

        FileClientConn* conn = (FileClientConn*)it_old->second;
        conn->OnTimer(cur_time);
    }
}

void FileTaskTimerCallback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
    uint64_t tick = get_tick_count();
    TransferTaskManager::GetInstance()->OnTimer(tick);
}

void InitializeFileClientConn()
{

#if 0
    char work_path[BUFSIZ];
    if(!getcwd(work_path, BUFSIZ)) {
        log("getcwd failed");
    } else {
        snprintf(g_current_save_path, BUFSIZ, "%s/offline_file", work_path);
    }
    
    log("save offline files to %s", g_current_save_path);
    
    int ret = mkdir(g_current_save_path, 0755);
    if ( (ret != 0) && (errno != EEXIST) ) {
        log("!!!mkdir failed to save offline files");
    }
#endif

    netlib_register_timer(FileClientConnTimerCallback, NULL, 1000);
    netlib_register_timer(FileTaskTimerCallback, NULL, 10000);
}

void FileClientConn::Close()
{
    log("close client, handle %d", m_handle);

    if (transfer_task_) {
        if (transfer_task_->GetTransMode() == FILE_TYPE_ONLINE) {
            transfer_task_->set_state(kTransferTaskStateInvalid);

            //            CImConn* conn = transfer_task_->GetOpponentConn(user_id_);
            //            if (conn) {
            //                FileClientConn* conn2 = reinterpret_cast<FileClientConn*>(conn);
            //                conn2->Close2();
            //            }
            // Close2();
        } else {
            if (transfer_task_->state() >= kTransferTaskStateUploadEnd) {
                transfer_task_->set_state(kTransferTaskStateWaitingDownload);
            }
        }
        transfer_task_->SetConnByUserID(user_id_, NULL);

        TransferTaskManager::GetInstance()->DeleteTransferTaskByConnClose(transfer_task_->task_id());

        // 关闭另一个连接
        //        if (transfer_task_->GetTransMode() == FILE_TYPE_ONLINE) {
        //        }
        transfer_task_ = NULL;
    }
    auth_ = false;

    if (m_handle != NETLIB_INVALID_HANDLE) {
        netlib_close(m_handle);
        g_file_client_conn_map.erase(m_handle);
    }

    //    if (user_id_ > 0) {
    //        g_file_user_map.erase(m_user_id);
    //        m_user_id = 0;
    //    }

    ReleaseRef();
}

#if 0
void FileClientConn::Close2() {
    log("close2 client, handle %d", m_handle);
    
    auth_ = false;

    if (transfer_task_) {
        if (transfer_task_->GetTransMode() == FILE_TYPE_ONLINE) {
        } else {
            if (transfer_task_->state() >= kTransferTaskStateUploadEnd) {
                transfer_task_->set_state(kTransferTaskStateWaitingDownload);
            }
        }
        transfer_task_->SetConnByUserID(user_id_, NULL);
//        TransferTaskManager::GetInstance()->DeleteTransferTaskByConnClose(transfer_task_->task_id());
//        
//        // 关闭另一个连接
//        if (transfer_task_->GetTransMode() == FILE_TYPE_ONLINE) {
//            CImConn* conn = transfer_task_->GetOpponentConn(user_id_);
//            if (conn) {
//                FileClientConn* conn2 = reinterpret_cast<FileClientConn*>(conn);
//                conn2->Close();
//            }
//        }
//        transfer_task_ = NULL;
    }
    auth_ = false;
    
    if (m_handle != NETLIB_INVALID_HANDLE) {
        netlib_close(m_handle);
        g_file_client_conn_map.erase(m_handle);
    }
    
    //    if (user_id_ > 0) {
    //        g_file_user_map.erase(m_user_id);
    //        m_user_id = 0;
    //    }
    
    ReleaseRef();
}
#endif

void FileClientConn::OnConnect(net_handle_t handle)
{
    /// yunfan modify 2014.8.7
    m_handle = handle;

    g_file_client_conn_map.insert(make_pair(handle, this));
    netlib_option(handle, NETLIB_OPT_SET_CALLBACK, (void*)imconn_callback);
    netlib_option(handle, NETLIB_OPT_SET_CALLBACK_DATA, (void*)&g_file_client_conn_map);

    uint32_t socket_buf_size = NETLIB_MAX_SOCKET_BUF_SIZE;
    netlib_option(handle, NETLIB_OPT_SET_SEND_BUF_SIZE, &socket_buf_size);
    netlib_option(handle, NETLIB_OPT_SET_RECV_BUF_SIZE, &socket_buf_size);
    /// yunfan modify end
}

void FileClientConn::OnClose()
{
    log("client onclose: handle=%d", m_handle);
    Close();
}

void FileClientConn::OnTimer(uint64_t curr_tick)
{

    if (transfer_task_ && transfer_task_->GetTransMode() == FILE_TYPE_ONLINE) {
        if (transfer_task_->state() == kTransferTaskStateInvalid) {
            log("Close another online conn, user_id=%d", user_id_);
            Close();
            return;
        }
    }
    // transfer_task_->set_state(kTransferTaskStateInvalid);

    if (curr_tick > m_last_recv_tick + CLIENT_TIMEOUT) {
        log("client timeout, user_id=%u", user_id_);
        Close();
    }
}

void FileClientConn::OnWrite()
{
    CImConn::OnWrite();
}

void FileClientConn::HandlePdu(CImPdu* pdu)
{
    switch (pdu->GetCommandId()) {
    case CID_OTHER_HEARTBEAT:
        _HandleHeartBeat(pdu);
        break;
    case CID_FILE_LOGIN_REQ:
        _HandleClientFileLoginReq(pdu);
        break;

    case CID_FILE_STATE:
        _HandleClientFileStates(pdu);
        break;
    case CID_FILE_PULL_DATA_REQ:
        _HandleClientFilePullFileReq(pdu);
        break;
    case CID_FILE_PULL_DATA_RSP:
        _HandleClientFilePullFileRsp(pdu);
        break;

    default:
        log("no such cmd id: %u", pdu->GetCommandId());
        break;
    }
}

void FileClientConn::_HandleHeartBeat(CImPdu* pdu) { SendPdu(pdu); }



// Client客户端（包括接受方与发送方）发起向file_server登录请求
void FileClientConn::_HandleClientFileLoginReq(CImPdu* pdu)
{
    IM::File::IMFileLoginReq login_req;
    CHECK_PB_PARSE_MSG(login_req.ParseFromArray(pdu->GetBodyData(), pdu->GetBodyLength()));

    uint32_t user_id = login_req.user_id();
    string task_id = login_req.task_id();
    IM::BaseDefine::ClientFileRole mode = login_req.file_role();

    log("Client login, user_id=%d, task_id=%s, file_role=%d", user_id, task_id.c_str(), mode);

    BaseTransferTask* transfer_task = NULL;

    bool rv = false;
    // 1.根据taskId查找对应的文件传输任务是否存在
    do {
        transfer_task = TransferTaskManager::GetInstance()->FindByTaskID(task_id);
        // 1-1 文件传输任务不存在（有可能是离线任务 or 异常）
        if (transfer_task == NULL) {
            if (mode == CLIENT_OFFLINE_DOWNLOAD) {
                // 如果文件不存在，但是传输模式为离线文件传输
                // 尝试从磁盘加载离线文件 有可能是文件服务器重启
                transfer_task = TransferTaskManager::GetInstance()->NewTransferTask(task_id, user_id);
                if (transfer_task == NULL) {
                    // 需要再次判断是否加载成功
                    log("Find task id failed, user_id=%u, taks_id=%s, mode=%d", user_id, task_id.c_str(), mode);
                    break;
                }
            } else {
                // 异常文件任务id
                log("Can't find task_id, user_id=%u, taks_id=%s, mode=%d", user_id, task_id.c_str(), mode);
                break;
            }
        }
        // 1-2 状态转换
        rv = transfer_task->ChangePullState(user_id, mode); // 状态转换
        if (!rv)
            break;

        // 1-3 Ok
        auth_ = true;
        transfer_task_ = transfer_task;
        user_id_ = user_id;

        // 1-4 设置任务的往来对应的连接conn 保证文件传输的接收端与发送端对应关系
        transfer_task->SetConnByUserID(user_id, this);
        rv = true;
    } while (0);

    // 2.生成状态之后 进行文件的在线传输
    IM::File::IMFileLoginRsp login_rsp;
    login_rsp.set_result_code(rv ? 0 : 1);
    login_rsp.set_task_id(task_id);
    ::SendMessageLite(this, SID_FILE, CID_FILE_LOGIN_RES, pdu->GetSeqNum(), &login_rsp);
    if (rv) {
        if (transfer_task->GetTransMode() == FILE_TYPE_ONLINE) {
            // 2-1 进行文件的在线传输
            if (transfer_task->state() == kTransferTaskStateWaitingTransfer) {
                // 通知接收端已经准备好了接受文件
                CImConn* conn = transfer_task_->GetToConn(); // 获取接收端conn
                if (conn)
                    _StatesNotify(CLIENT_FILE_PEER_READY, task_id, transfer_task_->from_user_id(), conn);
                else {
                    log("to_conn is close, close me!!!");
                    Close();
                }
                // _StatesNotify(CLIENT_FILE_PEER_READY, task_id, user_id, this);
                // transfer_task->StatesNotify(CLIENT_FILE_PEER_READY, task_id,
                // user_id_);
            }
        } else {
            // 2-2 进行文件的离线传输
            if (transfer_task->state() == kTransferTaskStateWaitingUpload) {
                OfflineTransferTask* offline = reinterpret_cast<OfflineTransferTask*>(transfer_task);
                IM::File::IMFilePullDataReq pull_data_req;
                pull_data_req.set_task_id(task_id);
                pull_data_req.set_user_id(user_id);
                pull_data_req.set_trans_mode(FILE_TYPE_OFFLINE);
                pull_data_req.set_offset(0);
                pull_data_req.set_data_size(offline->GetNextSegmentBlockSize());
                ::SendMessageLite(this, SID_FILE, CID_FILE_PULL_DATA_REQ, &pull_data_req);
                log("Pull Data Req");
            }
        }
    } else {
        Close();
    }
}



void FileClientConn::_HandleClientFileStates(CImPdu* pdu)
{
    if (!auth_ || !transfer_task_) {
        log("Recv a client_file_state, but auth is false");
        return;
    }

    IM::File::IMFileState file_state;
    CHECK_PB_PARSE_MSG(file_state.ParseFromArray(pdu->GetBodyData(), pdu->GetBodyLength()));

    string task_id = file_state.task_id();
    uint32_t user_id = file_state.user_id();
    uint32_t file_stat = file_state.state();

    log("Recv FileState, user_id=%d, task_id=%s, file_stat=%d", user_id, task_id.c_str(), file_stat);

    // FilePullFileRsp
    bool rv = false;
    do {
        // 检查user_id
        if (user_id != user_id_) {
            log("Received user_id valid, recv_user_id = %d, transfer_task.user_id = %d, user_id_ = %d", user_id, transfer_task_->from_user_id(), user_id_);
            break;
        }

        // 检查task_id
        if (transfer_task_->task_id() != task_id) {
            log("Received task_id valid, recv_task_id = %s, this_task_id = %s", task_id.c_str(), transfer_task_->task_id().c_str());
            break;
        }

        switch (file_stat) {
        case CLIENT_FILE_CANCEL:
        case CLIENT_FILE_DONE:
        case CLIENT_FILE_REFUSE: {
            CImConn* im_conn = transfer_task_->GetOpponentConn(user_id);
            if (im_conn) {
                im_conn->SendPdu(pdu);
                log("Task %s %d by user_id %d notify %d, erased", task_id.c_str(), file_stat, user_id, transfer_task_->GetOpponent(user_id));
            }
            // notify other client
            // CFileConn* pConn = (CFileConn*)t->GetOpponentConn(user_id);
            // pConn->SendPdu(pPdu);

            // TransferTaskManager::GetInstance()->DeleteTransferTask(task_id);

            rv = true;
            break;
        }

        default:
            log("Recv valid file_stat: file_state = %d, user_id=%d, task_id=%s", file_stat, user_id_, task_id.c_str());
            break;
        }

        // rv = true;
    } while (0);

    // if (!rv) {
    Close();
    // }

#if 0
    // 查找任务是否存在
    BaseTransferTask* transfer_task = TransferTaskManager::GetInstance()->FindByTaskID(task_id);
    switch (file_stat) {
        case CLIENT_FILE_CANCEL:
        case CLIENT_FILE_DONE:
        case CLIENT_FILE_REFUSE:
        {
            CImConn* im_conn = transfer_task->GetConnByUserID(user_id);
            if (im_conn) {
                im_conn->SendPdu(pdu);
                log("Task %s %d by user_id %d notify %d, erased", task_id.c_str(), file_stat, user_id, transfer_task->GetOpponent(user_id));
            }
            // notify other client
            // CFileConn* pConn = (CFileConn*)t->GetOpponentConn(user_id);
            // pConn->SendPdu(pPdu);
            
            TransferTaskManager::GetInstance()->DeleteTransferTask(task_id);
            break;
        }
            
        default:
            break;
    }
#endif
}


/**
 * 接受者发起文件接收请求 -> file_server -> 发送方
 * 接收端对文件接收请求的处理逻辑
 * 包括验证请求的有效性、根据不同的传输模式进行逻辑处理
 * 并通过网络通信将请求转发给发送方或发送文件数据给接收方
*/
void FileClientConn::_HandleClientFilePullFileReq(CImPdu* pdu)
{
    // 1.检查是否已进行身份验证auth_ 和 是否存在该传输任务transfer_task_
    // 如果身份验证失败 或 没有该项传输任务 直接返回false
    if (!auth_ || !transfer_task_) {
        log("Recv a client_file_state, but auth is false");
        return;
    }

    // 2.解析收到的request请求数据
    IM::File::IMFilePullDataReq pull_data_req;
    CHECK_PB_PARSE_MSG(pull_data_req.ParseFromArray(pdu->GetBodyData(), pdu->GetBodyLength()));
    uint32_t user_id = pull_data_req.user_id(); // 用户id
    string task_id = pull_data_req.task_id(); // 任务id
    uint32_t mode = pull_data_req.trans_mode(); // 传输模式
    uint32_t offset = pull_data_req.offset(); // 文件传输偏移量
    uint32_t datasize = pull_data_req.data_size(); // 文件数据

    log("Recv FilePullFileReq, user_id=%d, task_id=%s, file_role=%d, offset=%d, datasize=%d",
        user_id, task_id.c_str(), mode, offset, datasize);

    // 3.组装response响应消息 接收端 -> file_server -> 发送端
    IM::File::IMFilePullDataRsp pull_data_rsp;
    pull_data_rsp.set_result_code(1);
    pull_data_rsp.set_task_id(task_id);
    pull_data_rsp.set_user_id(user_id);
    pull_data_rsp.set_offset(offset);
    pull_data_rsp.set_file_data("");

    // BaseTransferTask* transfer_task = NULL;
    int rv = -1;

    /**
     * rv 用于表示操作的返回值 根据代码的逻辑其值可能不同
     * 在执行 DoPullFileRequest 函数时返回 -1 表示发生了错误 需要终止执行
     * 
     * rv != 0 文件传输未完成
     * rv == 1 文件传输已完成
     * 
     * 在离线传输模式下 如果 rv == 1 会调用_StatesNotify函数通知文件传输已经完成
     * 在在线传输模式下 不论 rv取值如何 都会通过对应的连接conn 将请求转发给发送方
    */
    do {
        // 4.检查用户ID和任务ID的有效性 包括传输任务的id是否匹配
        // 4-1 检查user_id用户是否匹配 文件传输请求的用户id 与 当前连接的用户id
        // user_id 是从IM::File::IMFilePullDataReq消息中解析出来的字段，表示文件传输请求的用户ID
        // user_id_ 是 FileClientConn 类的成员变量，表示当前连接的用户ID
        if (user_id != user_id_) {
            log("Received user_id valid, recv_user_id = %d, transfer_task.user_id = %d, user_id_ = %d",
                user_id, transfer_task_->from_user_id(), user_id_);
            break;
        }

        // 4-2 检查task_id任务是否匹配 文件传输请求中的taskId 与 当前连接的任务taskId
        // task_id 从收到的文件传输请求中解析出 是一个字符串，用于标识特定的文件传输任务
        // transfer_task_->task_id() 用于获取当前连接的传输任务的任务标识符 调用了当前连接所关联的传输任务对象
        if (transfer_task_->task_id() != task_id) {
            log("Received task_id valid, recv_task_id = %s, this_task_id = %s",
                task_id.c_str(), transfer_task_->task_id().c_str());
            break;
        }

        // 4-3 检查传输任务的目标用户id 是否与请求中的用户id匹配 如果不匹配则终止处理
        // user_id 从消息中解析出来的字段 表示文件传输请求的user_id
        // user_id 是否为 transfer_task.to_user_id
        if (!transfer_task_->CheckToUserID(user_id)) {
            log("user_id equal transfer_task.to_user_id, but user_id=%d, transfer_task.to_user_id=%d",
                user_id, transfer_task_->to_user_id());
            break;
        }

        // 5.调用传输任务的DoPullFileRequest函数处理文件拉取请求 根据传输模式的不同进行不同的逻辑处理
        // DoPullFileRequest函数 如果是在线模式则只是检测状态 如果是离线状态则该函数会将文件数据 存入pull_data_rsp中
        // 离线传输需要下载文件 在线传输从发送者拉数据
        rv = transfer_task_->DoPullFileRequest(user_id, offset, datasize, pull_data_rsp.mutable_file_data());
        if (rv == -1)
            break;

        pull_data_rsp.set_result_code(0);

        if (transfer_task_->GetTransMode() == FILE_TYPE_ONLINE) {
            // 5-1 在线传输模式
            // （1）设置消息序列号
            // 类型转换 将transfer_task_（BaseTransferTask* 类型）转换为（OnlineTransferTask* 类型）并赋值给 online变量
            // reinterpret_cast是 C++ 中的一种类型转换方式 它执行低级的类型转换，可以将一个指针或引用转换为不同类型的指针或引用 但是需要注意潜在的类型不匹配和未定义行为
            // 这里 transfer_task_ 是一个基类 BaseTransferTask 类型的指针, 而 OnlineTransferTask 是继承自 BaseTransferTask 的派生类。可以进行类型转换
            OnlineTransferTask* online = reinterpret_cast<OnlineTransferTask*>(transfer_task_);

            // 调用OnlineTransferTask类中的 SetSeqNum 成员函数
            // 将消息的序列号（SeqNum）设置给 OnlineTransferTask 对象，以便后续使用该序列号进行消息的处理和识别。
            online->SetSeqNum(pdu->GetSeqNum());

            // （2）文件在线传输 通过对应的连接conn 直接将请求转发给发送方
            CImConn* conn = transfer_task_->GetOpponentConn(user_id);
            if (conn) {
                conn->SendPdu(pdu);
                // SendMessageLite(conn, SID_FILE, CID_FILE_PULL_DATA_RSP,
                // pdu->GetSeqNum(), &pull_data_rsp);
            }
            // SendPdu(&pdu);
        } else {
            // 5-2 离线传输模式
            // （1）离线传输 直接通过当前连接this 发送响应消息
            SendMessageLite(this, SID_FILE, CID_FILE_PULL_DATA_RSP, pdu->GetSeqNum(), &pull_data_rsp);

            // （2）如果传输任务完成 rv == 1 则调用_StatesNotify函数通知状态变化
            if (rv == 1)
                _StatesNotify(CLIENT_FILE_DONE, task_id, transfer_task_->from_user_id(), this);
        }
    } while (0);

    // 5.如果传输过程中发生任何错误 则会关闭当前连接conn
    if (rv != 0)
        Close();
}


/**
 * file_server处理收到客户端发送的文件拉取响应（FilePullDataRsp）的情况
 * 发送者发起文件发送请求 -> file_server -> 接收方
 * 函数接收一个 CImPdu 对象作为参数，通过解析该对象的消息体，获取文件拉取的响应数据，并进行相应的处理
*/
void FileClientConn::_HandleClientFilePullFileRsp(CImPdu* pdu)
{
    // 1.首先进行认证和传输任务的检查，确保认证和传输任务对象存在。
    if (!auth_ || !transfer_task_) {
        log("auth is false");
        return;
    }

    // 2.解析响应数据 通过解析pdu对象的消息体，将数据解析为 IM::File::IMFilePullDataRsp 类型的 pull_data_rsp
    IM::File::IMFilePullDataRsp pull_data_rsp;
    CHECK_PB_PARSE_MSG(pull_data_rsp.ParseFromArray(pdu->GetBodyData(), pdu->GetBodyLength()));
    uint32_t user_id = pull_data_rsp.user_id(); // 用户id
    string task_id = pull_data_rsp.task_id(); // 任务id
    uint32_t offset = pull_data_rsp.offset(); // 偏移量offset
    // pull_data_rsp.file_data()是一个字符串对象
    // 由于 data_size 的类型是uint32_t 需要使用static_cast 进行类型转换 确保将字符串长度转换为正确的类型并存储在 data_size 变量中
    // data_size 会用于处理文件数据的大小，判断是否接收完整数据、计算数据偏移量等
    uint32_t data_size = static_cast<uint32_t>(pull_data_rsp.file_data().length()); // 数据大小
    const char* data = pull_data_rsp.file_data().data(); // 数据内容

    // log("Recv FilePullFileRsp, user_id=%d, task_id=%s, file_role=%d, offset=%d,
    // datasize=%d", user_id, task_id.c_str(), mode, offset, datasize);
    log("Recv FilePullFileRsp, task_id=%s, user_id=%u, offset=%u, data_size=%d",
        task_id.c_str(), user_id, offset, data_size);

    int rv = -1;
    do {
        // 3.进行传输检查
        //  3-1 检查user_id用户是否匹配 文件传输请求的用户id 与 当前连接的用户id
        //  user_id 是从IM::File::IMFilePullDataReq消息中解析出来的字段，表示文件传输请求的用户ID
        //  user_id_ 是 FileClientConn 类的成员变量，表示当前连接的用户ID
        if (user_id != user_id_) {
            log("Received user_id valid, recv_user_id = %d, transfer_task.user_id = %d, user_id_ = %d",
                user_id, transfer_task_->from_user_id(), user_id_);
            break;
        }

        // 3-2 检查task_id任务是否匹配 文件传输请求中的taskId 与 当前连接的任务taskId
        // task_id 从收到的文件传输请求中解析出 是一个字符串，用于标识特定的文件传输任务
        // transfer_task_->task_id() 用于获取当前连接的传输任务的任务标识符 调用了当前连接所关联的传输任务对象
        if (transfer_task_->task_id() != task_id) {
            log("Received task_id valid, recv_task_id = %s, this_task_id = %s",
                task_id.c_str(), transfer_task_->task_id().c_str());
            break;
        }

        // 4.针对不同的传输模式 进行不同的逻辑处理方案
        // DoPullFileRequest函数 如果是在线模式则只是检测状态 如果是离线状态则该函数会将文件数据 存入pull_data_rsp中
        rv = transfer_task_->DoRecvData(user_id, offset, data, data_size);
        if (rv == -1)
            break;

        if (transfer_task_->GetTransMode() == FILE_TYPE_ONLINE) {
            // 4-1 对于在线，直接转发
            // （1）设置消息序列号
            OnlineTransferTask* online = reinterpret_cast<OnlineTransferTask*>(transfer_task_);
            pdu->SetSeqNum(online->GetSeqNum());
            // online->SetSeqNum(pdu->GetSeqNum());

            // （2）直接转发消息给接收方
            CImConn* conn = transfer_task_->GetToConn();
            if (conn)
                conn->SendPdu(pdu);
        } else {
            // 4-2 对于离线文件 存入file_server
            if (rv == 1) {
                // （1）如果离线文件已经接受完毕 则通知接受者文件已经接受完 all packages recved
                _StatesNotify(CLIENT_FILE_DONE, task_id, user_id, this);
                // Close();
            } else {
                // （2）为传输完成则向 file_server文件服务器 再次发送请求继续拉取数据
                //  类型转换 transfer_task_ -> OfflineTransferTask*
                OfflineTransferTask* offline = reinterpret_cast<OfflineTransferTask*>(transfer_task_);

                IM::File::IMFilePullDataReq pull_data_req;
                pull_data_req.set_task_id(task_id);
                pull_data_req.set_user_id(user_id);
                pull_data_req.set_trans_mode(static_cast<IM::BaseDefine::TransferFileType>(offline->GetTransMode()));
                pull_data_req.set_offset(offline->GetNextOffset());
                pull_data_req.set_data_size(offline->GetNextSegmentBlockSize());

                ::SendMessageLite(this, SID_FILE, CID_FILE_PULL_DATA_REQ, &pull_data_req);
                // log("size not match");
            }
        }
    } while (0);

    // 5.如果返回值rv不为0，表示文件传输出错或离线上传已完成，关闭连接。
    if (rv != 0) {
        // -1，文件传输出错关闭
        //  1, 离线上传完成
        Close();
    }
}

int FileClientConn::_StatesNotify(int state, const std::string& task_id, uint32_t user_id, CImConn* conn)
{
    FileClientConn* file_client_conn = reinterpret_cast<FileClientConn*>(conn);

    IM::File::IMFileState file_msg;
    file_msg.set_state(static_cast<ClientFileState>(state));
    file_msg.set_task_id(task_id);
    file_msg.set_user_id(user_id);

    ::SendMessageLite(conn, SID_FILE, CID_FILE_STATE, &file_msg);

    log("notify to user %d state %d task %s", user_id, state, task_id.c_str());
    return 0;
}
