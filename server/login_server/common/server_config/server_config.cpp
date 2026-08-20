/**
 * @author: luochenhao
 * @email: lch2022fox@163.com
 * @time: Mon 04 May 2026 19:20:53 CST
 * @brief: 读取 login_server.conf 配置文件
*/

#include <teamtalk/imcore/string/string.h>
#include <teamtalk/imcore/config_reader/config_reader.h>

#include "server_config.h"

namespace teamtalk::login_server::common::server_config {

namespace ttconfig = teamtalk::imcore::config_reader;
namespace ttstring = teamtalk::imcore::string;

ServerConfig& ServerConfig::Instance() {
  static ServerConfig inst;
  return inst;
}

bool ServerConfig::LoadFromFile(const std::string& path) {
  ttconfig::CConfigReader config_file(path.c_str());

  msfs_url_ = config_file.GetConfigValue("msfs");
  discovery_ = config_file.GetConfigValue("discovery");

  if (msfs_url_.empty() || discovery_.empty()) {
    return false;
  }

  ttstring::str_explode(config_file.GetConfigValue("ClientListenIP"), ';', client_listen_addrs_);
  client_listen_port_ = static_cast<uint16_t>(config_file.GetUint32Value("ClientListenPort", 0));

  ttstring::str_explode(config_file.GetConfigValue("MsgServerListenIP"), ';', msg_server_listen_addrs_);
  msg_server_listen_port_ = static_cast<uint16_t>(config_file.GetUint32Value("MsgServerListenPort", 0));

  ttstring::str_explode(config_file.GetConfigValue("HttpListenIP"), ';', http_listen_addrs_);
  http_listen_port_ = static_cast<uint16_t>(config_file.GetUint32Value("HttpListenPort", 0));

  if (client_listen_addrs_.empty() || client_listen_port_ == 0 ||
      msg_server_listen_addrs_.empty() || msg_server_listen_port_ == 0 ||
      http_listen_addrs_.empty() || http_listen_port_ == 0) {
    return false;
  }

  return true;
}

}  // namespace teamtalk::login_server::common::server_config