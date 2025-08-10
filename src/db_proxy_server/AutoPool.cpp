/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: AutoPool.cpp
 Update Time: Wed 14 Jun 2023 22:58:54 CST
 brief:
*/

#include "DBPool.h"
#include "AutoPool.h"
#include "CachePool.h"

CAutoDB::CAutoDB(const char* pDBName, CDBConn** pDBConn)
{
    m_pDBConn = CDBManager::getInstance()->GetDBConn(pDBName);
    *pDBConn = m_pDBConn;
}

CAutoDB::~CAutoDB()
{
    if (m_pDBConn != NULL) {
        CDBManager::getInstance()->RelDBConn(m_pDBConn);
        m_pDBConn = NULL;
    }
}

CAutoCache::CAutoCache(const char* pCacheName, CacheConn** pCacheConn)
{
    m_pCacheConn = CacheManager::getInstance()->GetCacheConn(pCacheName);
    *pCacheConn = m_pCacheConn;
}

CAutoCache::~CAutoCache()
{
    if (m_pCacheConn != NULL) {
        CacheManager::getInstance()->RelCacheConn(m_pCacheConn);
        m_pCacheConn = NULL;
    }
}
