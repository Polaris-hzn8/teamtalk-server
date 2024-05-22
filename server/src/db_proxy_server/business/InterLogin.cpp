/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: InterLogin.cpp
 Update Time: Thu 15 Jun 2023 00:31:05 CST
 brief:
*/

#include "InterLogin.h"
#include "../DBPool.h"
#include "EncDec.h"

bool CInterLoginStrategy::doLogin(const std::string& strName, const std::string& strPass, IM::BaseDefine::UserInfo& user) {
    bool bRet = false;
    // 1.获取数据库连接对象 pDBConn
    CDBManager* pDBManger = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManger->GetDBConn("teamtalk_slave");

    if (pDBConn) {
        // 2.构造SQL查询语句，通过用户名strName查询符合条件的用户信息
        string strSql = "select * from IMUser where name='" + strName + "' and status=0";
        // 3.执行 SQL 查询并获取结果集 pResultSet
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if (pResultSet) {
            string strResult, strSalt;
            uint32_t nId, nGender, nDeptId, nStatus;
            string strNick, strAvatar, strEmail, strRealName, strTel, strDomain, strSignInfo;
            // 4.遍历结果集，提取出查询到的用户信息，包括用户的 ID、密码、盐值以及其他相关字段
            while (pResultSet->Next()) {
                nId = pResultSet->GetInt("id");
                strResult = pResultSet->GetString("password");
                strSalt = pResultSet->GetString("salt");

                strNick = pResultSet->GetString("nick");
                nGender = pResultSet->GetInt("sex");
                strRealName = pResultSet->GetString("name");
                strDomain = pResultSet->GetString("domain");
                strTel = pResultSet->GetString("phone");
                strEmail = pResultSet->GetString("email");
                strAvatar = pResultSet->GetString("avatar");
                nDeptId = pResultSet->GetInt("departId");
                nStatus = pResultSet->GetInt("status");
                strSignInfo = pResultSet->GetString("sign_info");
            }

            // 5.将输入的密码 strPass 和 混淆值拼接，然后计算MD5值得到 strOutPass
            string strInPass = strPass + strSalt;//md5(pwd) + salt
            char szMd5[33];
            CMd5::MD5_Calculate(strInPass.c_str(), strInPass.length(), szMd5);
            string strOutPass(szMd5);

            // 6.将计算得到的 strOutPass 与数据库查询到的密码进行比较，如果相等则表示登录验证成功
            if (strOutPass == strResult) {
                // 7.如果登录验证成功，将相应的用户信息设置到user对象中，并将返回值bRet设置为 true表示登录验证成功
                bRet = true;
                user.set_user_id(nId);
                user.set_user_nick_name(strNick);
                user.set_user_gender(nGender);
                user.set_user_real_name(strRealName);
                user.set_user_domain(strDomain);
                user.set_user_tel(strTel);
                user.set_email(strEmail);
                user.set_avatar_url(strAvatar);
                user.set_department_id(nDeptId);
                user.set_status(nStatus);
                user.set_sign_info(strSignInfo);
            }
            delete pResultSet;
        }
        pDBManger->RelDBConn(pDBConn);
    }
    return bRet;
}


