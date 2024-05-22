/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: im_conn_util.cpp
 Update Time: Mon 12 Jun 2023 23:47:56 CST
 brief: 
*/

#include "im_conn_util.h"

#include "base/imconn.h"
#include "base/ImPduBase.h"

/// @brief SendMessageLite函数重载
/// @param conn 要发送消息的连接对象
/// @param sid 消息的服务id
/// @param cid 消息的命令id
/// @param message 要发送的消息对象 使用Google Protocol Buffers库的MessageLite类
/// @return 发送消息的结果
int SendMessageLite(CImConn* conn, uint16_t sid, uint16_t cid, const ::google::protobuf::MessageLite* message) {
    CImPdu pdu;
    
    pdu.SetPBMsg(message);
    pdu.SetServiceId(sid);
    pdu.SetCommandId(cid);
    
    return conn->SendPdu(&pdu);
}

/// @brief SendMessageLite函数重载
/// @param conn 要发送消息的连接对象
/// @param sid 服务id
/// @param cid 命令id
/// @param seq_num 消息的序列号
/// @param message 要发送的消息对象 使用Google Protocol Buffers库的MessageLite类
/// @return 发送消息的结果
int SendMessageLite(CImConn* conn, uint16_t sid, uint16_t cid, uint16_t seq_num, const ::google::protobuf::MessageLite* message) {
    CImPdu pdu;
    
    pdu.SetPBMsg(message);
    pdu.SetServiceId(sid);
    pdu.SetCommandId(cid);
    pdu.SetSeqNum(seq_num);
    
    return conn->SendPdu(&pdu);
}

/// @brief SendMessageLite函数重载
/// @param conn 要发送消息的连接对象
/// @param sid 服务id
/// @param cid 命令id
/// @param seq_num 消息的序列号
/// @param error 消息的错误码
/// @param message 要发送的消息对象 使用Google Protocol Buffers库的MessageLite类
/// @return 发送消息的结果
int SendMessageLite(CImConn* conn, uint16_t sid, uint16_t cid, uint16_t seq_num, uint16_t error, const ::google::protobuf::MessageLite* message) {
    CImPdu pdu;
    
    pdu.SetPBMsg(message);
    pdu.SetServiceId(sid);
    pdu.SetCommandId(cid);
    pdu.SetSeqNum(seq_num);
    pdu.SetError(error);
    
    return conn->SendPdu(&pdu);
}


