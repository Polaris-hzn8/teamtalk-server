/**
 * @author: luochenhao
 * @email: lch2022fox@163.com
 * @time: Mon 04 May 2026 19:20:53 CST
 * @brief: 读取 login_server.conf 配置文件
*/

#include <teamtalk/imcore/config_reader/config_reader.h>

#include "server_config.h"

namespace teamtalk::login_server::common::server_config {

using ttconfig = teamtalk::imcore::config_reader::CConfigReader;

ServerConfig& ServerConfig::Instance() {
  static ServerConfig inst;
  return inst;
}

bool ServerConfig::LoadFromFile(const std::string& path) {
  ttconfig config_file(path.c_str());

  msfs_url_ = config_file.GetConfigValue("msfs");
  discovery_ = config_file.GetConfigValue("discovery");

  if (msfs_url_.empty() || discovery_.empty()) {
    return false;
  }

  client_listen_eps_ = config_file.ReadNumberedEndpoints("ClientListenIP", "ClientListenPort");
  msg_server_listen_eps_ = config_file.ReadNumberedEndpoints("MsgServerListenIP", "MsgServerListenPort");
  http_listen_eps_ = config_file.ReadNumberedEndpoints("HttpListenIP", "HttpListenPort");

  if (client_listen_eps_.empty() || msg_server_listen_eps_.empty() || http_listen_eps_.empty()) {
    return false;
  }

  return true;
}

}  // namespace teamtalk::login_server::common::server_config