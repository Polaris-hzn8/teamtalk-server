/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: im_conn_util.cpp
 Update Time: Mon 12 Jun 2023 23:47:56 CST
 brief: 
*/

#include "imconn.h"
#include "ImPduBase.h"
#include "im_conn_util.h"

int SendMessageLite(CImConn* conn, uint16_t sid, uint16_t cid, const ::google::protobuf::MessageLite* message)
{
    CImPdu pdu;    
    pdu.SetPBMsg(message);
    pdu.SetServiceId(sid);
    pdu.SetCommandId(cid);
    return conn->SendPdu(&pdu);
}

int SendMessageLite(CImConn* conn, uint16_t sid, uint16_t cid, uint16_t seq_num, const ::google::protobuf::MessageLite* message)
{
    CImPdu pdu;    
    pdu.SetPBMsg(message);
    pdu.SetServiceId(sid);
    pdu.SetCommandId(cid);
    pdu.SetSeqNum(seq_num);
    return conn->SendPdu(&pdu);
}

int SendMessageLite(CImConn* conn, uint16_t sid, uint16_t cid, uint16_t seq_num, uint16_t error, const ::google::protobuf::MessageLite* message)
{
    CImPdu pdu;    
    pdu.SetPBMsg(message);
    pdu.SetServiceId(sid);
    pdu.SetCommandId(cid);
    pdu.SetSeqNum(seq_num);
    pdu.SetError(error);
    return conn->SendPdu(&pdu);
}
