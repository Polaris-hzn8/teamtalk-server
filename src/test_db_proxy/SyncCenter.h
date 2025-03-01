/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: SyncCenter.h
 Update Time: Thu 15 Jun 2023 01:04:22 CST
 brief:
*/

#ifndef __CACHEMANAGER_H__
#define __CACHEMANAGER_H__

#include <list>
#include <map>

#include "Condition.h"
#include "IM.BaseDefine.pb.h"
#include "ImPduBase.h"
#include "Lock.h"
#include "ostype.h"
#include "public_define.h"

class CSyncCenter {
public:
    static CSyncCenter* getInstance();

    uint32_t getLastUpdate()
    {
        CAutoLock auto_lock(&last_update_lock_);
        return m_nLastUpdate;
    }
    uint32_t getLastUpdateGroup()
    {
        CAutoLock auto_lock(&last_update_lock_);
        return m_nLastUpdateGroup;
    }
    string getDeptName(uint32_t nDeptId);
    void startSync();
    void stopSync();
    void init();
    void updateTotalUpdate(uint32_t nUpdated);

private:
    void updateLastUpdateGroup(uint32_t nUpdated);

    CSyncCenter();
    ~CSyncCenter();
    static void* doSyncGroupChat(void* arg);

private:
    void getDept(uint32_t nDeptId, DBDeptInfo_t** pDept);
    DBDeptMap_t* m_pDeptInfo;

    static CSyncCenter* m_pInstance;
    uint32_t m_nLastUpdateGroup;
    uint32_t m_nLastUpdate;

    CCondition* m_pCondGroupChat;
    CLock* m_pLockGroupChat;
    static bool m_bSyncGroupChatRuning;
    bool m_bSyncGroupChatWaitting;
#ifdef _WIN32
    DWORD m_nGroupChatThreadId;
#else
    pthread_t m_nGroupChatThreadId;
#endif
    CLock last_update_lock_;
};

#endif /*defined(__CACHEMANAGER_H__) */
