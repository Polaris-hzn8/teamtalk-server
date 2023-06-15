/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ipparser.cpp
 Update Time: Thu 15 Jun 2023 00:44:59 CST
 brief:
*/

#include "ipparser.h"

IpParser::IpParser()
{
}

IpParser::~IpParser()
{
}

bool IpParser::isTelcome(const char* pIp)
{
    if (!pIp) {
        return false;
    }
    CStrExplode strExp((char*)pIp, '.');
    if (strExp.GetItemCnt() != 4) {
        return false;
    }
    return true;
}
