/**
 * @author: luochenhao
 * @email: lch2022fox@163.com
 * @time: Mon 04 May 2026 19:20:53 CST
 * @brief: 读取 msg_server.conf 配置文件
 */

#include <teamtalk/sbase/global_define.h>
#include <teamtalk/imcore/string/string.h>
#include <teamtalk/imcore/config_reader/config_reader.h>

#include "server_config.h"

namespace teamtalk::msg_server::common::server_config {

namespace ttconfig = teamtalk::imcore::config_reader;
namespace ttstring = teamtalk::imcore::string;

ServerConfig& ServerConfig::Instance() {
  static ServerConfig inst;
  return inst;
}

bool ServerConfig::LoadFromFile(const std::string& path) {
  ttconfig::CConfigReader config_file(path.c_str());

  ttstring::str_explode(config_file.GetConfigValue("ListenIP"), ';', listen_addrs_);

  listen_port_ = static_cast<uint16_t>(config_file.GetUint32Value("ListenPort", 0));

  ip_addr1_ = config_file.GetConfigValue("IpAddr1");
  ip_addr2_ = config_file.GetConfigValue("IpAddr2");

  aes_key_ = config_file.GetConfigValue("aesKey");

  max_conn_cnt_ = config_file.GetUint32Value("MaxConnCnt", 0);
  concurrent_db_conn_cnt_ = config_file.GetUint32Value("ConcurrentDBConnCnt", DEFAULT_CONCURRENT_DB_CONN_CNT);

  db_servers_ = config_file.ReadNumberedEndpoints("DBServerIP", "DBServerPort");
  login_servers_ = config_file.ReadNumberedEndpoints("LoginServerIP", "LoginServerPort");
  route_servers_ = config_file.ReadNumberedEndpoints("RouteServerIP", "RouteServerPort");
  push_servers_ = config_file.ReadNumberedEndpoints("PushServerIP", "PushServerPort");
  file_servers_ = config_file.ReadNumberedEndpoints("FileServerIP", "FileServerPort");

  if (!db_servers_.empty() && concurrent_db_conn_cnt_ > 0) {
    uint32_t expanded_count = static_cast<uint32_t>(db_servers_.size()) * concurrent_db_conn_cnt_;
    expanded_db_servers_.resize(expanded_count);
    for (uint32_t i = 0; i < expanded_count; i++) {
      uint32_t idx = i / concurrent_db_conn_cnt_;
      expanded_db_servers_[i] = db_servers_[idx];
    }
  }

  if (listen_addrs_.empty() || listen_port_ == 0 || ip_addr1_.empty() || ip_addr2_.empty()) {
    return false;
  }

  if (aes_key_.empty() || aes_key_.length() != 32) {
    return false;
  }

  if (db_servers_.size() < 2) {
    return false;
  }

  return true;
}

}  // namespace teamtalk::msg_server::common::server_config