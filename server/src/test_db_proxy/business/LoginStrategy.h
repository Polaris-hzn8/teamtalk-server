/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: LoginStrategy.h
 Update Time: Thu 15 Jun 2023 01:06:58 CST
 brief:
*/

#ifndef __LOGINSTRATEGY_H__
#define __LOGINSTRATEGY_H__

#include <iostream>

#include "IM.BaseDefine.pb.h"

class CLoginStrategy {
public:
    virtual bool doLogin(const std::string& strName, const std::string& strPass, IM::BaseDefine::UserInfo& user) = 0;
};

#endif /*defined(__LOGINSTRATEGY_H__) */
