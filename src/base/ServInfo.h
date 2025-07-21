/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ServInfo.h
 Update Time: Tue 13 Jun 2023 14:55:47 CST
 brief: 
    该头文件提供了一些函数模板和结构体 用于管理服务器的连接和重新连接
    使用这些函数和结构体，可以方便地初始化服务器连接、检查并重新连接服务器以及重置服务器连接
    同时，提供了一个函数用于从配置文件中读取服务器配置信息
*/

#ifndef SERVINFO_H_
#define SERVINFO_H_

#include "util.h"
#include "imconn.h"
#include "ConfigFileReader.h"

#define MAX_RECONNECT_CNT 64    //最大重连次数
#define MIN_RECONNECT_CNT 4     //最小重连次数

//服务器信息结构体
typedef struct {
    string      server_ip;      //IP
    uint16_t    server_port;    //端口
    uint32_t    idle_cnt;       //空闲计数
    uint32_t    reconnect_cnt;  //重连计数
    CImConn*    serv_conn;      //服务器连接对象
} serv_info_t;

// 初始化服务器列表
template <class T>
void serv_init(serv_info_t* server_list, uint32_t server_count)
{
    for (uint32_t i = 0; i < server_count; i++) {
        T* pConn = new T();
        // Connect
        pConn->Connect(server_list[i].server_ip.c_str(), server_list[i].server_port, i);
        server_list[i].serv_conn = pConn;
        server_list[i].idle_cnt = 0;
        server_list[i].reconnect_cnt = MIN_RECONNECT_CNT / 2;
    }
}

// 服务器连接断开重连
template <class T>
void serv_check_reconnect(serv_info_t* server_list, uint32_t server_count)
{
    T* pConn;
    for (uint32_t i = 0; i < server_count; i++) {
        pConn = (T*)server_list[i].serv_conn;
        if (!pConn) {
            server_list[i].idle_cnt++;
            if (server_list[i].idle_cnt >= server_list[i].reconnect_cnt) {
                pConn = new T();
                pConn->Connect(server_list[i].server_ip.c_str(), server_list[i].server_port, i);
                server_list[i].serv_conn = pConn;
            }
        }
    }
}

// 重置服务器连接对象
template <class T>
void serv_reset(serv_info_t* server_list, uint32_t server_count, uint32_t serv_idx)
{
    if (serv_idx >= server_count)
        return;
    server_list[serv_idx].serv_conn = NULL;
    server_list[serv_idx].idle_cnt = 0;
    server_list[serv_idx].reconnect_cnt *= 2;
    if (server_list[serv_idx].reconnect_cnt > MAX_RECONNECT_CNT) {
        server_list[serv_idx].reconnect_cnt = MIN_RECONNECT_CNT;
    }
}

//读取服务器配置信息
serv_info_t* read_server_config(
    sCConfigFileReader* config_file,
    const char* server_ip_format,
    const char* server_port_format,
    uint32_t& server_count);

#endif
