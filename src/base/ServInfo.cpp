/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ServInfo.cpp
 Update Time: Tue 13 Jun 2023 14:56:01 CST
 brief: 
*/

#include "ServInfo.h"

/// @brief 从配置文件中读取服务器配置信息并返回服务器信息结构体数组
/// @param config_file 指向配置文件读取器对象的指针
/// @param server_ip_format 服务器IP地址的配置项格式字符串
/// @param server_port_format 服务器端口号的配置项格式字符串
/// @param server_count 用于存储读取到的服务器数量
/// @return 
serv_info_t* read_server_config(CConfigFileReader* config_file,
								const char* server_ip_format,
								const char* server_port_format,
								uint32_t& server_count) {
	
	// 1.定义用于构建配置项名称的字符数组
	char server_ip_key[64];
	char server_port_key[64];

	server_count = 0;

	// 2.通过循环遍历配置文件，逐个获取服务器ip地址和端口号port的配置项，直到遇到配置项不存在的情况为止
	while (true) {
		// 2-1.使用sprintf函数根据格式字符串和索引 构建配置项名称
		sprintf(server_ip_key, "%s%d", server_ip_format, server_count + 1);
		sprintf(server_port_key, "%s%d", server_port_format, server_count + 1);

		// 2-2.调用GetConfigName函数获取配置项的值
		char* server_ip_value = config_file->GetConfigName(server_ip_key);
		char* server_port_value = config_file->GetConfigName(server_port_key);

		// 2-3.遇到配置项不存在的情况退出循环
		if (!server_ip_value || !server_port_value) break;
		server_count++;
	}

	// 如果server_count为0，表示配置文件中没有服务器配置信息，此时返回NULL
	if (server_count == 0) return NULL;

	// 3.动态分配一个serv_info_t类型的数组，大小为server_count，用于存储服务器信息
	serv_info_t* server_list = new serv_info_t [server_count];

	// 4.再次通过循环遍历配置文件，将每个服务器的IP地址和端口号存储到相应的服务器信息结构体中
	for (uint32_t i = 0; i < server_count; i++) {
		// 4-1.使用sprintf函数根据格式字符串和索引 构建配置项名称 
		// IP地址配置项名称存储在server_ip_key中
		// 端口号配置项名称存储在server_port_key中
		sprintf(server_ip_key, "%s%d", server_ip_format, i + 1);
		sprintf(server_port_key, "%s%d", server_port_format, i + 1);

		// 4-2.调用GetConfigName函数获取配置项的值
		char* server_ip_value = config_file->GetConfigName(server_ip_key);
		char* server_port_value = config_file->GetConfigName(server_port_key);

		// 4-3.将获取到的服务器IP地址和端口号分别赋值给相应的服务器信息结构体中的server_ip和server_port成员
		server_list[i].server_ip = server_ip_value;
		server_list[i].server_port = atoi(server_port_value);
	}

	// 5.返回服务器信息结构体数组
	return server_list;
}

