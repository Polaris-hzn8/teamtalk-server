/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: AutoPool.h
 Update Time: Wed 14 Jun 2023 22:59:06 CST
 brief: 以自动化的方式管理mysql连接和redis缓存连接
*/

#ifndef __AUTOPOOl_H__
#define __AUTOPOOl_H__

class CDBConn;
class CacheConn;

// 数据库连接管理
class CAutoDB
{
public:
    CAutoDB(const char* pDBName, CDBConn** pDBConn);
    ~CAutoDB();
private:
    CDBConn* m_pDBConn;
};

// 缓存连接管理
class CAutoCache
{
    CAutoCache(const char* pCacheName, CacheConn** pCacheConn);
    ~CAutoCache();
private:
    CacheConn* m_pCacheConn;
};

#endif
