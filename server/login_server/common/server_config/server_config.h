
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
#include <utility>
#include <vector>
#include <cstdint>

namespace teamtalk::login_server::common::server_config {

using Endpoint = std::pair<std::string, uint16_t>;

class ServerConfig {
 public:
  static ServerConfig& Instance();

  bool LoadFromFile(const std::string& path);

  const std::vector<Endpoint>& client_listen_endpoints() const { return client_listen_eps_; }
  const std::vector<Endpoint>& msg_server_listen_endpoints() const { return msg_server_listen_eps_; }
  const std::vector<Endpoint>& http_listen_endpoints() const { return http_listen_eps_; }

  const std::string& msfs_url() const { return msfs_url_; }
  const std::string& discovery() const { return discovery_; }

 private:
  ServerConfig() = default;
  ServerConfig(const ServerConfig&) = delete;
  ServerConfig& operator=(const ServerConfig&) = delete;

  std::string msfs_url_;
  std::string discovery_;

  std::vector<Endpoint> client_listen_eps_;
  std::vector<Endpoint> msg_server_listen_eps_;
  std::vector<Endpoint> http_listen_eps_;
};

}  // namespace teamtalk::login_server::common::server_config

#endif  // TEAMTALK_LOGIN_SERVER_COMMON_SERVER_CONFIG_SERVER_CONFIG_H_