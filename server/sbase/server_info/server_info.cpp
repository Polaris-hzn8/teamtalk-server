/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ServInfo.cpp
 Update Time: Tue 13 Jun 2023 14:56:01 CST
 brief:
*/

#include <cstdio>
#include "server_info.h"

namespace teamtalk::sbase::server_info {

// 读取服务器配置信息
serv_info_t* read_server_config(ttconfig::CConfigReader* config_file,
                                const char* server_ip_format,
                                const char* server_port_format,
                                uint32_t& server_count) {
  char server_ip_key[64];
  char server_port_key[64];

  server_count = 0;

  while (true) {
    snprintf(server_ip_key, sizeof(server_ip_key), "%s%d", server_ip_format, server_count + 1);
    snprintf(server_port_key, sizeof(server_port_key), "%s%d", server_port_format, server_count + 1);

    std::string server_ip_value = config_file->GetConfigValue(server_ip_key);
    std::string server_port_value = config_file->GetConfigValue(server_port_key);

    // 配置项不存在
    if (server_ip_value.empty() || server_port_value.empty()) {
      break;
    }

    server_count++;
  }

  if (server_count == 0) {
    return nullptr;
  }

  serv_info_t* server_list = new serv_info_t[server_count];
  for (uint32_t i = 0; i < server_count; i++) {
    snprintf(server_ip_key, sizeof(server_ip_key), "%s%d", server_ip_format, i + 1);
    snprintf(server_port_key, sizeof(server_port_key), "%s%d", server_port_format, i + 1);

    server_list[i].server_ip = config_file->GetConfigValue(server_ip_key);
    server_list[i].server_port = config_file->GetUint32Value(server_port_key, 0);
  }
  return server_list;
}

}  // namespace teamtalk::sbase::server_info
