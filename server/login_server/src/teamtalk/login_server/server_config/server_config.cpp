/**
 * @author: luochenhao
 * @email: lch2022fox@163.com
 * @time: Mon 04 May 2026 19:20:53 CST
 * @brief: 读取 login_server.conf 配置文件
*/

#include "server_config.h"
#include <teamtalk/imcore/config_reader/config_reader.h>

namespace teamtalk::login_server {

LoginServerConfig& LoginServerConfig::Instance() {
  static LoginServerConfig inst;
  return inst;
}

bool LoginServerConfig::LoadFromFile(const std::string& path) {
  teamtalk::imcore::config_reader::CConfigFileReader config_file(path.c_str());

  client_listen_ip_ = config_file.GetConfigValue("ClientListenIP");
  http_listen_ip_ = config_file.GetConfigValue("HttpListenIP");
  std::string str_http_port = config_file.GetConfigValue("HttpPort");
  msg_server_listen_ip_ = config_file.GetConfigValue("MsgServerListenIP");
  std::string str_msg_server_port = config_file.GetConfigValue("MsgServerPort");
  msfs_url_ = config_file.GetConfigValue("msfs");
  discovery_ = config_file.GetConfigValue("discovery");

  if (msg_server_listen_ip_.empty() || str_msg_server_port.empty() ||
      http_listen_ip_.empty() || str_http_port.empty() ||
      msfs_url_.empty() || discovery_.empty()) {
    return false;
  }

  client_port_ = static_cast<uint16_t>(config_file.GetUint32Value("ClientPort", 0));
  msg_server_port_ = static_cast<uint16_t>(config_file.GetUint32Value("MsgServerPort", 0));
  http_port_ = static_cast<uint16_t>(config_file.GetUint32Value("HttpPort", 0));
  return true;
}

}  // namespace teamtalk::login_server
