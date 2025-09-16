/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: SyncCenter.h
 Update Time: Wed 14 Jun 2023 23:07:13 CST
 brief:
*/

#ifndef __CACHEMANAGER_H__
#define __CACHEMANAGER_H__

#include <map>
#include <list>

#include "Lock.h"
#include "ostype.h"
#include "Condition.h"
#include "ImPduBase.h"
#include "public_define.h"
#include "IM.BaseDefine.pb.h"

class CSyncCenter
{
public:
    static CSyncCenter* getInstance();

    uint32_t getLastUpdate() {
        CAutoLock auto_lock(&last_update_lock_);
        return m_nLastUpdate;
    }
    uint32_t getLastUpdateGroup() {
        CAutoLock auto_lock(&last_update_lock_);
        return m_nLastUpdateGroup;
    }

    std::string getDeptName(uint32_t nDeptId);
    void init();
    void startSync();
    void stopSync();
    void updateTotalUpdate(uint32_t nUpdated);
    void updateLastUpdateGroup(uint32_t nUpdated);

private:
    CSyncCenter();
    ~CSyncCenter();
    static void* doSyncGroupChat(void* arg);

private:
    void getDept(uint32_t nDeptId, DBDeptInfo_t** pDept);
    DBDeptMap_t* m_pDeptInfo;

    static CSyncCenter* m_pInstance;
    static bool         m_bSyncGroupChatRuning;

    uint32_t        m_nLastUpdateGroup;
    uint32_t        m_nLastUpdate;

    CLock*          m_pLockGroupChat;
    CCondition*     m_pCondGroupChat;
    CLock           last_update_lock_;

    bool            m_bSyncGroupChatWaitting;
#ifdef _WIN32
    DWORD           m_nGroupChatThreadId;
#else
    pthread_t       m_nGroupChatThreadId;
#endif
};

#endif /*defined(__CACHEMANAGER_H__) */
