/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: InterLogin.cpp
 Update Time: Thu 15 Jun 2023 00:31:05 CST
 brief:
*/

#include "EncDec.h"
#include "InterLogin.h"
#include "../DBPool.h"

bool CInterLoginStrategy::doLogin(const std::string& strName, const std::string& strPass, IM::BaseDefine::UserInfo& user)
{
    bool bRet = false;
    CDBManager* pDBManger = CDBManager::getInstance();
    CDBConn* pDBConn = pDBManger->GetDBConn("teamtalk_slave");

    if (pDBConn) {
        std::string strSql = "select * from IMUser where name='" + strName + "' and status=0";
        CResultSet* pResultSet = pDBConn->ExecuteQuery(strSql.c_str());
        if (pResultSet) {
            std::string strResult, strSalt;
            uint32_t nId, nGender, nDeptId, nStatus;
            std::string strNick, strAvatar, strEmail, strRealName, strTel, strDomain, strSignInfo;
            while (pResultSet->Next()) {
                /* 结果集遍历 提取出查询到的用户信息 */
                // 用户id、密码、盐值以及其他相关字段
                strResult = pResultSet->GetString("password");
                strSalt = pResultSet->GetString("salt");

                nId = pResultSet->GetInt("id");
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

            // md5(pwd + salt)
            std::string strInPass = strPass + strSalt;
            char szMd5[33] = { 0 };
            CMd5::MD5_Calculate(strInPass.c_str(), strInPass.length(), szMd5);
            std::string strOutPass(szMd5);

            // 与数据库中查询的密码进行比较 相等则表示登录验证通过
            if (strOutPass == strResult) {
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


