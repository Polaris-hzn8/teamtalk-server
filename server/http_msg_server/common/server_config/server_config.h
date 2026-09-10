
/**
 * @author: luochenhao
 * @email: lch2022fox@163.com
 * @brief: http_server 进程级配置单例
 *    由 LoadFromFile 从 http_msg_server.conf 加载
 *    其它模块通过 Instance() 只读访问
 */

#ifndef TEAMTALK_HTTP_SERVER_COMMON_SERVER_CONFIG_SERVER_CONFIG_H_
#define TEAMTALK_HTTP_SERVER_COMMON_SERVER_CONFIG_SERVER_CONFIG_H_

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace teamtalk::http_server::common::server_config {

using ServerEndpoint = std::pair<std::string, uint16_t>;

class ServerConfig {
 public:
  static ServerConfig& Instance();

  bool LoadFromFile(const std::string& path);

  const std::vector<std::string>& listen_addresses() const { return listen_addrs_; }
  uint16_t listen_port() const { return listen_port_; }

  uint32_t concurrent_db_conn_cnt() const { return concurrent_db_conn_cnt_; }

  const std::vector<ServerEndpoint>& db_servers() const { return db_servers_; }
  const std::vector<ServerEndpoint>& expanded_db_servers() const { return expanded_db_servers_; }
  const std::vector<ServerEndpoint>& route_servers() const { return route_servers_; }

 private:
  ServerConfig() = default;
  ServerConfig(const ServerConfig&) = delete;
  ServerConfig& operator=(const ServerConfig&) = delete;

  std::vector<std::string> listen_addrs_;
  uint16_t listen_port_ = 0;

  uint32_t concurrent_db_conn_cnt_ = 0;

  std::vector<ServerEndpoint> db_servers_;
  std::vector<ServerEndpoint> expanded_db_servers_;
  std::vector<ServerEndpoint> route_servers_;
};

}  // namespace teamtalk::http_server::common::server_config

#endif  // TEAMTALK_HTTP_SERVER_COMMON_SERVER_CONFIG_SERVER_CONFIG_H_