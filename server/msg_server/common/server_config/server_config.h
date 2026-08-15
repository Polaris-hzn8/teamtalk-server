
/**
 * @author: luochenhao
 * @email: lch2022fox@163.com
 * @time: Mon 04 May 2026 19:18:32 CST
 * @brief: 进程级配置单例：由 LoadFromFile 从 msg_server.conf 加载
 *    其它模块通过 Instance() 只读访问
 *    后续新增配置项：加成员、在 LoadFromFile 里赋值、提供 getter 方法
*/

#ifndef TEAMTALK_MSG_SERVER_COMMON_SERVER_CONFIG_SERVER_CONFIG_H_
#define TEAMTALK_MSG_SERVER_COMMON_SERVER_CONFIG_SERVER_CONFIG_H_

#include <string>
#include <vector>
#include <cstdint>

namespace teamtalk::msg_server::common::server_config {

class ServerConfig {
 public:
  static ServerConfig& Instance();

  /** 读取配置文件；缺必填项时返回 false，且不修改已有字段（便于重试时可改为先清空） */
  bool LoadFromFile(const std::string& path);

  const std::string& client_listen_ip() const { return client_listen_ip_; }
  uint16_t client_port() const { return client_port_; }

  const std::string& http_listen_ip() const { return http_listen_ip_; }
  uint16_t http_port() const { return http_port_; }

  const std::string& msg_server_listen_ip() const { return msg_server_listen_ip_; }
  uint16_t msg_server_port() const { return msg_server_port_; }

  /** ClientListenIP */
  const std::vector<std::string>& client_listen_addresses() const { return client_listen_addrs_; }
  /** MsgServerListenIP */
  const std::vector<std::string>& msg_server_listen_addresses() const { return msg_server_listen_addrs_; }
  /** HttpListenIP */
  const std::vector<std::string>& http_listen_addresses() const { return http_listen_addrs_; }

  const std::string& msfs_url() const { return msfs_url_; }
  const std::string& discovery() const { return discovery_; }

 private:
  ServerConfig() = default;
  ServerConfig(const ServerConfig&) = delete;
  ServerConfig& operator=(const ServerConfig&) = delete;

  std::string client_listen_ip_;
  std::string http_listen_ip_;
  std::string msg_server_listen_ip_;
  uint16_t client_port_ = 0;
  uint16_t http_port_ = 0;
  uint16_t msg_server_port_ = 0;
  std::string msfs_url_;
  std::string discovery_;

  std::vector<std::string> client_listen_addrs_;
  std::vector<std::string> msg_server_listen_addrs_;
  std::vector<std::string> http_listen_addrs_;
};

}  // namespace teamtalk::msg_server::common::server_config

#endif  // TEAMTALK_MSG_SERVER_COMMON_SERVER_CONFIG_SERVER_CONFIG_H_
