
/**
 * @author: luochenhao
 * @email: lch2022fox@163.com
 * @time: Mon 04 May 2026 19:18:32 CST
 * @brief: 进程级配置单例：由 LoadFromFile 从 login_server.conf 加载
 *    其它模块通过 Instance() 只读访问
 *    后续新增配置项：加成员、在 LoadFromFile 里赋值、提供 getter 方法
*/

#ifndef TEAMTALK_LOGIN_SERVER_COMMON_SERVER_CONFIG_SERVER_CONFIG_H_
#define TEAMTALK_LOGIN_SERVER_COMMON_SERVER_CONFIG_SERVER_CONFIG_H_

#include <string>
#include <vector>
#include <cstdint>

namespace teamtalk::login_server::common::server_config {

class ServerConfig {
 public:
  static ServerConfig& Instance();

  bool LoadFromFile(const std::string& path);

  const std::vector<std::string>& client_listen_addresses() const { return client_listen_addrs_; }
  uint16_t client_listen_port() const { return client_listen_port_; }

  const std::vector<std::string>& msg_server_listen_addresses() const { return msg_server_listen_addrs_; }
  uint16_t msg_server_listen_port() const { return msg_server_listen_port_; }

  const std::vector<std::string>& http_listen_addresses() const { return http_listen_addrs_; }
  uint16_t http_listen_port() const { return http_listen_port_; }

  const std::string& msfs_url() const { return msfs_url_; }
  const std::string& discovery() const { return discovery_; }

 private:
  ServerConfig() = default;
  ServerConfig(const ServerConfig&) = delete;
  ServerConfig& operator=(const ServerConfig&) = delete;

  std::string msfs_url_;
  std::string discovery_;

  std::vector<std::string> client_listen_addrs_;
  uint16_t client_listen_port_ = 0;

  std::vector<std::string> msg_server_listen_addrs_;
  uint16_t msg_server_listen_port_ = 0;

  std::vector<std::string> http_listen_addrs_;
  uint16_t http_listen_port_ = 0;
};

}  // namespace teamtalk::login_server::common::server_config

#endif  // TEAMTALK_LOGIN_SERVER_COMMON_SERVER_CONFIG_SERVER_CONFIG_H_