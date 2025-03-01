/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: config_util.cpp
 Update Time: Thu 15 Jun 2023 00:39:56 CST
 brief:
*/

#include "file_server/config_util.h"

void ConfigUtil::AddAddress(const char* ip, uint16_t port)
{
    IM::BaseDefine::IpAddr addr;
    addr.set_ip(ip);
    addr.set_port(port);
    addrs_.push_back(addr);
}
