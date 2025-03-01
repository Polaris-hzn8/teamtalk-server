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

#include "ConfigFileReader.h"
#include "imconn.h"
#include "util.h"

#define MAX_RECONNECT_CNT 64 //最大重连次数
#define MIN_RECONNECT_CNT 4 //最小重连次数

//服务器信息结构体
typedef struct {
    string server_ip;//服务器IP
    uint16_t server_port;//端口
    uint32_t idle_cnt;//空闲计数
    uint32_t reconnect_cnt;//重连计数
    CImConn* serv_conn;//服务器连接对象
} serv_info_t;


/**
 * 这个函数通过模板参数T可以适配不同类型的服务器连接对象，例如CImConn
 * 其遍历服务器列表，为每个服务器创建连接对象并初始化相关参数，以便后续使用
*/
/// @brief 函数模板定义 用于初始化服务器连接对象
/// @tparam T 服务器连接对象的类型
/// @param server_list 指向服务器信息结构体数组的指针
/// @param server_count 服务器信息结构体数组的大小
template <class T>
void serv_init(serv_info_t* server_list, uint32_t server_count) {
    //遍历服务器信息结构体数组
    for (uint32_t i = 0; i < server_count; i++) {
        //对于每个服务器，创建一个类型为T的服务器连接对象
        T* pConn = new T();
        //调用服务器连接对象的Connect方法，传入服务器的IP地址、端口号和序号i
        pConn->Connect(server_list[i].server_ip.c_str(), server_list[i].server_port, i);
        //将服务器连接对象赋值给相应的服务器信息结构体的serv_conn成员
        server_list[i].serv_conn = pConn;
        //将服务器信息结构体的idle_cnt设置为0
        server_list[i].idle_cnt = 0;
        //将服务器信息结构体的reconnect_cnt设置为MIN_RECONNECT_CNT / 2
        server_list[i].reconnect_cnt = MIN_RECONNECT_CNT / 2;
    }
}

/**
 * 这个函数通过模板参数T可以适配不同类型的服务器连接对象，例如CImConn。
 * 遍历服务器列表，检查每个服务器的连接状态，如果连接断开且达到了重连的条件，就创建新的连接对象并进行重新连接
*/
/// @brief 函数模板定义 用于检查服务器连接对象是否需要重新连接
/// @tparam T 服务器连接对象的类型
/// @param server_list 指向服务器信息结构体数组的指针
/// @param server_count 服务器信息结构体数组的大小
template <class T>
void serv_check_reconnect(serv_info_t* server_list, uint32_t server_count) {
    T* pConn;
    //遍历服务器信息结构体数组
    for (uint32_t i = 0; i < server_count; i++) {
        //获取服务器连接对象指针，并将其转换为类型为T*
        pConn = (T*)server_list[i].serv_conn;
        //如果连接对象为空指针，表示连接已断开 尝试进行重新连接
        if (!pConn) {
            //将服务器信息结构体的idle_cnt加1
            server_list[i].idle_cnt++;
            //如果idle_cnt达到了重连的次数阈值
            if (server_list[i].idle_cnt >= server_list[i].reconnect_cnt) {
                //创建一个类型为T的服务器连接对象
                pConn = new T();
                //调用连接对象的Connect方法，传入服务器的IP地址、端口号和序号（i）
                pConn->Connect(server_list[i].server_ip.c_str(), server_list[i].server_port, i);
                //将连接对象赋值给相应的服务器信息结构体的serv_conn成员
                server_list[i].serv_conn = pConn;
            }
        }
    }
}

/**
 * 这个函数通过模板参数T可以适配不同类型的服务器连接对象，例如CImConn。
 * 根据给定的服务器索引重置相应服务器的连接对象状态，包括将连接对象设为NULL、重置空闲计数器、调整重连次数等
*/
/// @brief 函数模板定义 用于重置服务器连接对象的状态
/// @tparam T 服务器连接对象的类型
/// @param server_list 指向服务器信息结构体数组的指针
/// @param server_count 服务器信息结构体数组的大小
/// @param serv_idx 要重置的服务器的索引
template <class T>
void serv_reset(serv_info_t* server_list, uint32_t server_count, uint32_t serv_idx) {
    //首先检查要重置的服务器的索引是否超出了数组的范围，如果超出则直接返回
    if (serv_idx >= server_count) return;
    //将服务器信息结构体的serv_conn成员设置为NULL，表示连接已断开
    server_list[serv_idx].serv_conn = NULL;
    //将服务器信息结构体的idle_cnt重置为0，表示空闲计数器归零
    server_list[serv_idx].idle_cnt = 0;
    //将服务器信息结构体的reconnect_cnt乘以2，表示下次重连的次数将增加一倍
    server_list[serv_idx].reconnect_cnt *= 2;
    //如果重连次数超过了最大重连次数阈值，则将重连次数重置为最小重连次数阈值
    if (server_list[serv_idx].reconnect_cnt > MAX_RECONNECT_CNT) {
        server_list[serv_idx].reconnect_cnt = MIN_RECONNECT_CNT;
    }
}

//从配置文件中读取服务器配置信息
serv_info_t* read_server_config(CConfigFileReader* config_file,
    const char* server_ip_format,
    const char* server_port_format,
    uint32_t& server_count);

#endif
