/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: im_conn_util.h
 Update Time: Mon 12 Jun 2023 23:47:47 CST
 brief: 用于发送消息的辅助函数
*/

#ifndef BASE_IM_CONN_UTIL_H_
#define BASE_IM_CONN_UTIL_H_

#include "base/ostype.h"

namespace google { namespace protobuf {
    class MessageLite;
}}

//类前置声明，文件中使用了CImConn类
class CImConn;

/**
 * 这些函数提供了便捷的方式来发送使用Google Protocol Buffers库定义的消息对象
 * 通过传入连接对象、服务id、命令id等参数，可以将消息对象 转换为字节流并发送给对应的连接
*/
int SendMessageLite(CImConn* conn, uint16_t sid, uint16_t cid, const ::google::protobuf::MessageLite* message);
int SendMessageLite(CImConn* conn, uint16_t sid, uint16_t cid, uint16_t seq_num, const ::google::protobuf::MessageLite* message);
int SendMessageLite(CImConn* conn, uint16_t sid, uint16_t cid, uint16_t seq_num, uint16_t error, const ::google::protobuf::MessageLite* message);

#endif
