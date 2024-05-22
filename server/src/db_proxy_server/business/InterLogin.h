/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: InterLogin.h
 Update Time: Thu 15 Jun 2023 00:31:11 CST
 brief:
*/

#ifndef __INTERLOGIN_H__
#define __INTERLOGIN_H__

#include "LoginStrategy.h"

class CInterLoginStrategy : public CLoginStrategy {
public:
    virtual bool doLogin(const std::string& strName, const std::string& strPass, IM::BaseDefine::UserInfo& user);
};

#endif /*defined(__INTERLOGIN_H__) */
