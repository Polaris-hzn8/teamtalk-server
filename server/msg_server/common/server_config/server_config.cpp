/**
 * @author: luochenhao
 * @email: lch2022fox@163.com
 * @time: Mon 04 May 2026 19:20:53 CST
 * @brief: 读取 login_server.conf 配置文件
*/

#include <common/server_config/server_config.h>
#include <teamtalk/imcore/config_reader/config_reader.h>

namespace teamtalk::login_server::common::server_config {

namespace {
/** 按分号拆分字符串，跳过空片段 */
void split_by_semicolon(const std::string& raw, std::vector<std::string>* out) {
  out->clear();
  if (raw.empty()) {
    return;
  }
  size_t pos = 0;
  while (pos < raw.size()) {
    size_t sep = raw.find(';', pos);
    const size_t end = (sep == std::string::npos) ? raw.size() : sep;
    if (end > pos) {
      out->emplace_back(raw, pos, end - pos);
    }
    pos = (sep == std::string::npos) ? raw.size() : sep + 1;
  }
}
}  // namespace

ServerConfig& ServerConfig::Instance() {
  static ServerConfig inst;
  return inst;
}

bool ServerConfig::LoadFromFile(const std::string& path) {
  teamtalk::imcore::config_reader::CConfigReader config_file(path.c_str());

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

  split_by_semicolon(client_listen_ip_, &client_listen_addrs_);
  split_by_semicolon(msg_server_listen_ip_, &msg_server_listen_addrs_);
  split_by_semicolon(http_listen_ip_, &http_listen_addrs_);

  if (msg_server_listen_addrs_.empty() || http_listen_addrs_.empty()) {
    return false;
  }

  return true;
}

}  // namespace teamtalk::login_server::common::server_config
