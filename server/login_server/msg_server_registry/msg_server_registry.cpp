
#include <mutex>
#include "msg_server_registry.h"

namespace teamtalk::login_server::msg_server_registry {

MsgServerRegistry& MsgServerRegistry::Instance() {
  static MsgServerRegistry instance;
  return instance;
}

bool MsgServerRegistry::Empty() const {
  std::shared_lock<std::shared_mutex> lock(mutex_);
  return msg_serv_info_map_.empty();
}

// load balancing
bool MsgServerRegistry::PickLeastLoaded(msg_serv_info_t* out) const {
  if (!out) {
    return false;
  }
  std::shared_lock<std::shared_mutex> lock(mutex_);
  const msg_serv_info_t* target = nullptr;
  uint32_t min_user_cnt = static_cast<uint32_t>(-1);
  for (MsgServInfoMap::const_iterator it = msg_serv_info_map_.begin(); it != msg_serv_info_map_.end(); ++it) {
    const msg_serv_info_t* info = it->second.get();
    if (info->cur_conn_cnt < info->max_conn_cnt && info->cur_conn_cnt < min_user_cnt) {
      target = info;
      min_user_cnt = info->cur_conn_cnt;
    }
  }
  if (!target) {
    return false;
  }
  *out = *target;
  return true;
}

bool MsgServerRegistry::UpdateConnCount(uint32_t handle, int32_t delta, msg_serv_info_t* out) {
  if (!out) {
    return false;
  }
  std::unique_lock<std::shared_mutex> lock(mutex_);
  MsgServInfoMap::iterator it = msg_serv_info_map_.find(handle);
  if (it == msg_serv_info_map_.end()) {
    return false;
  }
  msg_serv_info_t* info = it->second.get();
  if (delta >= 0) {
    info->cur_conn_cnt += static_cast<uint32_t>(delta);
  } else {
    const uint32_t dec = static_cast<uint32_t>(-delta);
    info->cur_conn_cnt = (info->cur_conn_cnt > dec) ? (info->cur_conn_cnt - dec) : 0;
  }
  *out = *info;
  return true;
}

void MsgServerRegistry::Upsert(uint32_t handle, std::unique_ptr<msg_serv_info_t> info) {
  std::unique_lock<std::shared_mutex> lock(mutex_);
  msg_serv_info_map_[handle] = std::move(info);
}

std::unique_ptr<msg_serv_info_t> MsgServerRegistry::RemoveByHandle(uint32_t handle) {
  std::unique_lock<std::shared_mutex> lock(mutex_);
  MsgServInfoMap::iterator it = msg_serv_info_map_.find(handle);
  if (it == msg_serv_info_map_.end()) {
    return nullptr;
  }
  std::unique_ptr<msg_serv_info_t> info = std::move(it->second);
  msg_serv_info_map_.erase(it);
  return info;
}

}  // namespace teamtalk::login_server::msg_server_registry
