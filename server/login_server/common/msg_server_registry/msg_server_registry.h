#ifndef TEAMTALK_LOGIN_SERVER_COMMON_MSG_SERVER_REGISTRY_MSG_SERVER_REGISTRY_H_
#define TEAMTALK_LOGIN_SERVER_COMMON_MSG_SERVER_REGISTRY_MSG_SERVER_REGISTRY_H_

#include <map>
#include <memory>
#include <string>
#include <cstdint>
#include <shared_mutex>

namespace teamtalk::login_server::common::msg_server_registry {

typedef struct {
  std::string ip_addr1;   // 电信IP
  std::string ip_addr2;   // 网通IP
  uint16_t port;          // 端口
  uint32_t max_conn_cnt;  // 最大连接数
  uint32_t cur_conn_cnt;  // 当前连接数
  std::string hostname;   // 消息服务器的主机名
} msg_serv_info_t;

class MsgServerRegistry {
 public:
  using MsgServInfoMap = std::map<uint32_t, std::unique_ptr<msg_serv_info_t>>;

  static MsgServerRegistry& Instance();

  bool Empty() const;
  bool PickLeastLoaded(msg_serv_info_t* out) const;
  bool UpdateConnCount(uint32_t handle, int32_t delta, msg_serv_info_t* out);
  void Upsert(uint32_t handle, std::unique_ptr<msg_serv_info_t> info);
  std::unique_ptr<msg_serv_info_t> RemoveByHandle(uint32_t handle);

 private:
  MsgServerRegistry() = default;

 private:
  MsgServInfoMap msg_serv_info_map_;
  mutable std::shared_mutex mutex_;
};

}  // namespace teamtalk::login_server::common::msg_server_registry

#endif  // TEAMTALK_LOGIN_SERVER_COMMON_MSG_SERVER_REGISTRY_MSG_SERVER_REGISTRY_H_
