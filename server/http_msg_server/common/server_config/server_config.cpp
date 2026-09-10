/**
 * @author: luochenhao
 * @email: lch2022fox@163.com
 * @brief: 读取 http_msg_server.conf 配置文件
 */

#include <teamtalk/sbase/global_define.h>
#include <teamtalk/imcore/string/string.h>
#include <teamtalk/imcore/config_reader/config_reader.h>

#include "server_config.h"

namespace teamtalk::http_server::common::server_config {

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

  concurrent_db_conn_cnt_ = config_file.GetUint32Value("ConcurrentDBConnCnt", DEFAULT_CONCURRENT_DB_CONN_CNT);

  db_servers_ = config_file.ReadNumberedEndpoints("DBServerIP", "DBServerPort");
  route_servers_ = config_file.ReadNumberedEndpoints("RouteServerIP", "RouteServerPort");

  if (!db_servers_.empty() && concurrent_db_conn_cnt_ > 0) {
    uint32_t expanded_count = static_cast<uint32_t>(db_servers_.size()) * concurrent_db_conn_cnt_;
    expanded_db_servers_.resize(expanded_count);
    for (uint32_t i = 0; i < expanded_count; i++) {
      uint32_t idx = i / concurrent_db_conn_cnt_;
      expanded_db_servers_[i] = db_servers_[idx];
    }
  }

  if (listen_addrs_.empty() || listen_port_ == 0) {
    return false;
  }

  return true;
}

}  // namespace teamtalk::http_server::common::server_config