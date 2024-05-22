/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: AutoPool.h
 Update Time: Thu 15 Jun 2023 01:03:02 CST
 brief:
*/

#ifndef __AUTOPOOl_H__
#define __AUTOPOOl_H__

class CDBConn;
class CacheConn;

class CAutoDB {
public:
    CAutoDB(const char* pDBName, CDBConn** pDBConn);
    ~CAutoDB();

private:
    CDBConn* m_pDBConn;
};

class CAutoCache {
    CAutoCache(const char* pCacheName, CacheConn** pCacheConn);
    ~CAutoCache();

private:
    CacheConn* m_pCacheConn;
};
#endif /*defined(__AUTOPOOl_H__) */
