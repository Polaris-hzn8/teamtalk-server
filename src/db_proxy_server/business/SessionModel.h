/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: SessionModel.h
 Update Time: Thu 15 Jun 2023 00:32:49 CST
 brief:
*/

#ifndef __SESSIONMODEL_H__
#define __SESSIONMODEL_H__

#include "IM.BaseDefine.pb.h"
#include "ImPduBase.h"

class CSessionModel {
public:
    static CSessionModel* getInstance();
    ~CSessionModel() { }

    void getRecentSession(uint32_t userId, uint32_t lastTime, list<IM::BaseDefine::ContactSessionInfo>& lsContact);
    uint32_t getSessionId(uint32_t nUserId, uint32_t nPeerId, uint32_t nType, bool isAll);
    bool updateSession(uint32_t nSessionId, uint32_t nUpdateTime);
    bool removeSession(uint32_t nSessionId);
    uint32_t addSession(uint32_t nUserId, uint32_t nPeerId, uint32_t nType);

private:
    CSessionModel() {};
    void fillSessionMsg(uint32_t nUserId, list<IM::BaseDefine::ContactSessionInfo>& lsContact);

private:
    static CSessionModel* m_pInstance;
};

#endif /*defined(__SESSIONMODEL_H__) */
