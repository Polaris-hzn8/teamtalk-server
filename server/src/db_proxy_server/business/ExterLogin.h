/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ExterLogin.h
 Update Time: Thu 15 Jun 2023 00:29:40 CST
 brief:
*/

#ifndef __EXTERLOGIN_H__
#define __EXTERLOGIN_H__

#include "LoginStrategy.h"

class CExterLoginStrategy : public CLoginStrategy {
public:
    virtual bool doLogin(const std::string& strName, const std::string& strPass, IM::BaseDefine::UserInfo& user);
};
#endif /*defined(__EXTERLOGIN_H__) */
