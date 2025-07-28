/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: im_conn_util.h
 Update Time: Mon 12 Jun 2023 23:47:47 CST
 brief: 消息发送相关函数
*/

#ifndef BASE_IM_CONN_UTIL_H_
#define BASE_IM_CONN_UTIL_H_

#include "base/common/ostype.h"

namespace google {
    namespace protobuf {
        class MessageLite;
    }
}
class CImConn;

// 将消息对象转换为字节流 并发送给对应的连接
int SendMessageLite(CImConn* conn, uint16_t sid, uint16_t cid, const ::google::protobuf::MessageLite* message);
int SendMessageLite(CImConn* conn, uint16_t sid, uint16_t cid, uint16_t seq_num, const ::google::protobuf::MessageLite* message);
int SendMessageLite(CImConn* conn, uint16_t sid, uint16_t cid, uint16_t seq_num, uint16_t error, const ::google::protobuf::MessageLite* message);

#endif
