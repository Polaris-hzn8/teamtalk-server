/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: netlib.h
 Update Time: Wed 14 Jun 2023 08:47:55 CST
 brief: 
 	主要用于处理tcp连接 实现了一个网络库
	对外提供了调用的api ，它封装了CEventDispatch
*/

#ifndef __NETLIB_H__
#define __NETLIB_H__

#include "ostype.h"

//定义了一些网络操作的选项常量 NETLIB_OPT_*
#define NETLIB_OPT_SET_CALLBACK			1	
#define NETLIB_OPT_SET_CALLBACK_DATA	2
#define NETLIB_OPT_GET_REMOTE_IP		3
#define NETLIB_OPT_GET_REMOTE_PORT		4
#define NETLIB_OPT_GET_LOCAL_IP			5
#define NETLIB_OPT_GET_LOCAL_PORT		6
#define NETLIB_OPT_SET_SEND_BUF_SIZE	7
#define NETLIB_OPT_SET_RECV_BUF_SIZE	8

//
#define NETLIB_MAX_SOCKET_BUF_SIZE		(128 * 1024)

#ifdef __cplusplus
extern "C" {
#endif

//初始化网络库
int netlib_init();
//销毁网络库
int netlib_destroy();

//监听指定的服务器 IP 地址和端口
int netlib_listen(
		const char*	server_ip, 
		uint16_t	port,
		callback_t	callback,
		void*		callback_data);
//连接到指定的服务器 IP 地址和端口
net_handle_t netlib_connect(
		const char*	server_ip,
		uint16_t	port,
		callback_t	callback,
		void*		callback_data);

//发送数据到指定的网络句柄
int netlib_send(net_handle_t handle, void* buf, int len);

//从指定的网络句柄接收数据
int netlib_recv(net_handle_t handle, void* buf, int len);

//关闭指定的网络句柄
int netlib_close(net_handle_t handle);

//设置或获取网络句柄的选项
int netlib_option(net_handle_t handle, int opt, void* optval);

//注册定时器回调函数
int netlib_register_timer(callback_t callback, void* user_data, uint64_t interval);

//删除定时器回调函数
int netlib_delete_timer(callback_t callback, void* user_data);

//添加循环回调函数
int netlib_add_loop(callback_t callback, void* user_data);

//启动网络事件循环，处理网络事件和定时器事件
void netlib_eventloop(uint32_t wait_timeout = 100);

//停止网络事件循环
void netlib_stop_event();

//检查网络事件循环是否在运行
bool netlib_is_running();


#ifdef __cplusplus
}
#endif

#endif

