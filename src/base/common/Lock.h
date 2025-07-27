/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: Lock.h
 Update Time: Mon 12 Jun 2023 17:43:23 CST
 brief: 封装的锁机制
*/

#ifndef __LOCK_H__
#define __LOCK_H__

#include "ostype.h"

class CLock
{
public:
    CLock();
    virtual ~CLock();

    void lock();
    void unlock();
    pthread_mutex_t& getMutex() { return m_lock; }
#ifndef _WIN32
    virtual bool try_lock();
#endif
private:
#ifdef _WIN32
	CRITICAL_SECTION    m_critical_section;
#else
    pthread_mutex_t     m_lock;
#endif
};

class CAutoLock
{
public:
    CAutoLock(CLock* pLock);
    virtual ~CAutoLock();
private:
    CLock* m_pLock;
};

////////////////////////////////////////////////////////// 读写锁
#ifndef _WIN32
class CRWLock
{
public:
    CRWLock();
    virtual ~CRWLock();

    void rlock();
    void wlock();
    void unlock();
    bool try_rlock();
    bool try_wlock();
private:
    pthread_rwlock_t m_lock;
};

class CAutoRWLock
{
public:
    CAutoRWLock(CRWLock* pLock, bool bRLock = true);
    virtual ~CAutoRWLock();
private:
    CRWLock* m_pLock;
};
#endif

#endif
