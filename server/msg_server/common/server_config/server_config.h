
/**
 * @author: Polaris_hzn8
 * @email: lch2022fox@163.com
 * @time: Mon 04 May 2026 19:18:32 CST
 * @brief: msg_server 进程级配置单例，封装 msg_server.conf 的全部读取逻辑
 *    使用方通过 ServerConfig::Instance() 访问配置，无需在 main 中手动解析
 */

#ifndef TEAMTALK_MSG_SERVER_COMMON_SERVER_CONFIG_SERVER_CONFIG_H_
#define TEAMTALK_MSG_SERVER_COMMON_SERVER_CONFIG_SERVER_CONFIG_H_

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace teamtalk::msg_server::common::server_config {

using ServerEndpoint = std::pair<std::string, uint16_t>;

class ServerConfig {
 public:
  static ServerConfig& Instance();

  bool LoadFromFile(const std::string& path);

  const std::vector<std::string>& listen_addresses() const { return listen_addrs_; }
  uint16_t listen_port() const { return listen_port_; }

  const std::string& ip_addr1() const { return ip_addr1_; }
  const std::string& ip_addr2() const { return ip_addr2_; }

  const std::string& aes_key() const { return aes_key_; }

  uint32_t max_conn_cnt() const { return max_conn_cnt_; }
  uint32_t concurrent_db_conn_cnt() const { return concurrent_db_conn_cnt_; }

  const std::vector<ServerEndpoint>& db_servers() const { return db_servers_; }
  const std::vector<ServerEndpoint>& expanded_db_servers() const { return expanded_db_servers_; }
  const std::vector<ServerEndpoint>& login_servers() const { return login_servers_; }
  const std::vector<ServerEndpoint>& route_servers() const { return route_servers_; }
  const std::vector<ServerEndpoint>& push_servers() const { return push_servers_; }
  const std::vector<ServerEndpoint>& file_servers() const { return file_servers_; }

 private:
  ServerConfig() = default;
  ServerConfig(const ServerConfig&) = delete;
  ServerConfig& operator=(const ServerConfig&) = delete;

  std::vector<std::string> listen_addrs_;
  uint16_t listen_port_ = 0;

  std::string ip_addr1_;
  std::string ip_addr2_;

  std::string aes_key_;

  uint32_t max_conn_cnt_ = 0;
  uint32_t concurrent_db_conn_cnt_ = 0;

  std::vector<ServerEndpoint> db_servers_;
  std::vector<ServerEndpoint> expanded_db_servers_;
  std::vector<ServerEndpoint> login_servers_;
  std::vector<ServerEndpoint> route_servers_;
  std::vector<ServerEndpoint> push_servers_;
  std::vector<ServerEndpoint> file_servers_;
};

}  // namespace teamtalk::msg_server::common::server_config

#endif  // TEAMTALK_MSG_SERVER_COMMON_SERVER_CONFIG_SERVER_CONFIG_H_