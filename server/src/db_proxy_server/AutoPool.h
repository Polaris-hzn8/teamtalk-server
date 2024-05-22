/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: AutoPool.h
 Update Time: Wed 14 Jun 2023 22:59:06 CST
 brief:
*/

#ifndef __AUTOPOOl_H__
#define __AUTOPOOl_H__

class CDBConn;
class CacheConn;

/**
 * 以自动化的方式管理mysql数据和redis缓存数据库
*/

/**
 * 该类用于管理数据库连接
*/
class CAutoDB {
public:
    //构造函数接受一个数据库名称和一个指向CDBConn对象的双指针 
    CAutoDB(const char* pDBName, CDBConn** pDBConn);
    //负责清理数据库连接
    ~CAutoDB();

private:
    CDBConn* m_pDBConn;
};

/**
 * 该类用于管理缓存连接
*/
class CAutoCache {
    //构造函数接受一个缓存名称和一个指向CacheConn对象的双指针
    CAutoCache(const char* pCacheName, CacheConn** pCacheConn);
    //析构函数负责清理缓存连接
    ~CAutoCache();

private:
    CacheConn* m_pCacheConn;
};
#endif
