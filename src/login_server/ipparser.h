/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ipparser.h
 Update Time: Thu 15 Jun 2023 00:45:07 CST
 brief:
*/

#ifndef _IPPARSER_H_
#define _IPPARSER_H_

#include "util.h"

class IpParser {
public:
    IpParser();
    virtual ~IpParser();
    bool isTelcome(const char* ip);//指示该IP地址是否属于电信公司
};

#endif
