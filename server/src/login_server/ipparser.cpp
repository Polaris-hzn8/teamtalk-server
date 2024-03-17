/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ipparser.cpp
 Update Time: Thu 15 Jun 2023 00:44:59 CST
 brief:
*/

#include "ipparser.h"

IpParser::IpParser() {}

IpParser::~IpParser() {}

//判断给定的IP地址是否属于电信公司
bool IpParser::isTelcome(const char* pIp) {
    //pIp为NULL则直接返回false
    if (!pIp) return false;
    //使用CStrExplode类将IP地址按照点号.进行分割，并计算分割后的子字符串数量
    CStrExplode strExp((char*)pIp, '.');
    //分割后的子字符串数量不等于4则返回false，表示不是一个有效的IP地址
    if (strExp.GetItemCnt() != 4) return false;
    return true;
}
