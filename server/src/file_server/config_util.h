/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: config_util.h
 Update Time: Thu 15 Jun 2023 00:40:05 CST
 brief:
*/

#ifndef FILE_SERVER_CONFIG_UTIL_H_
#define FILE_SERVER_CONFIG_UTIL_H_

#include <list>

#include "base/pb/protocol/IM.BaseDefine.pb.h"
#include "base/singleton.h"

class ConfigUtil : public Singleton<ConfigUtil> {
public:
    ~ConfigUtil() { }

    void AddAddress(const char* ip, uint16_t port);
    const std::list<IM::BaseDefine::IpAddr>& GetAddressList() const
    {
        return addrs_;
    }

    void SetTaskTimeout(uint32_t timeout) { task_timeout_ = timeout; }
    uint32_t GetTaskTimeout() const { return task_timeout_; }

private:
    friend class Singleton<ConfigUtil>;

    ConfigUtil()
        : task_timeout_(3600)
    {
    }

    std::list<IM::BaseDefine::IpAddr> addrs_;
    uint32_t task_timeout_;
};

#endif /* defined(FILE_SERVER_CONFIG_UTIL_H_) */
