/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: UserModel.h
 Update Time: Thu 15 Jun 2023 00:33:16 CST
 brief:
*/

#ifndef __USERMODEL_H__
#define __USERMODEL_H__

#include "ImPduBase.h"
#include "public_define.h"
#include "IM.BaseDefine.pb.h"

class CUserModel
{
public:
    static CUserModel* getInstance();
    ~CUserModel();
    void getChangedId(uint32_t& nLastTime, std::list<uint32_t>& lsIds);
    void getUsers(std::list<uint32_t> lsIds, std::list<IM::BaseDefine::UserInfo>& lsUsers);
    bool getUser(uint32_t nUserId, DBUserInfo_t& cUser);

    bool updateUser(DBUserInfo_t& cUser);
    bool insertUser(DBUserInfo_t& cUser);
    //    void getUserByNick(const list<string>& lsNicks, list<IM::BaseDefine::UserInfo>& lsUsers);
    void clearUserCounter(uint32_t nUserId, uint32_t nPeerId, IM::BaseDefine::SessionType nSessionType);
    void setCallReport(uint32_t nUserId, uint32_t nPeerId, IM::BaseDefine::ClientType nClientType);

    bool updateUserSignInfo(uint32_t user_id, const std::string& sign_info);
    bool getUserSingInfo(uint32_t user_id, std::string* sign_info);
    bool updatePushShield(uint32_t user_id, uint32_t shield_status);
    bool getPushShield(uint32_t user_id, uint32_t* shield_status);

private:
    CUserModel();

private:
    static CUserModel* m_pInstance;
};

#endif /*defined(__USERMODEL_H__) */
