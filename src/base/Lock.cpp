/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: Lock.cpp
 Update Time: Mon 12 Jun 2023 17:43:03 CST
 brief: 锁机制的封装
*/

#include "Lock.h"

////////////////////////////CLock/////////////////////////////////////
CLock::CLock() {
#ifdef _WIN32
    InitializeCriticalSection(&m_critical_section);
#else
    pthread_mutex_init(&m_lock, NULL);
#endif
}

CLock::~CLock() {
#ifdef _WIN32
    DeleteCriticalSection(&m_critical_section);
#else
    pthread_mutex_destroy(&m_lock);
#endif
}

void CLock::lock() {
#ifdef _WIN32
    EnterCriticalSection(&m_critical_section);
#else
    pthread_mutex_lock(&m_lock);
#endif
}

void CLock::unlock() {
#ifdef _WIN32
    LeaveCriticalSection(&m_critical_section);
#else
    pthread_mutex_unlock(&m_lock);
#endif
}

#ifndef _WIN32
bool CLock::try_lock() {
    // pthread_mutex_trylock(&m_lock)是一个函数调用 用于尝试获取互斥锁
    // 如果成功获取到互斥锁返回值为0 否则返回其他非零值
    return pthread_mutex_trylock(&m_lock) == 0;
}
#endif

////////////////////////////CAutoLock/////////////////////////////////////
CAutoLock::CAutoLock(CLock* pLock) {
    m_pLock = pLock;
    if (m_pLock != NULL) m_pLock->lock();
}

CAutoLock::~CAutoLock() {
    if (NULL != m_pLock) m_pLock->unlock();
}


////////////////////////////CRWLock/////////////////////////////////////
#ifndef _WIN32
CRWLock::CRWLock() {
    pthread_rwlock_init(&m_lock, NULL);
}

CRWLock::~CRWLock() {
    pthread_rwlock_destroy(&m_lock);
}

void CRWLock::rlock() {
    pthread_rwlock_rdlock(&m_lock);
}

void CRWLock::wlock() {
    pthread_rwlock_wrlock(&m_lock);
}

void CRWLock::unlock() {
    pthread_rwlock_unlock(&m_lock);
}

bool CRWLock::try_rlock() {
    return pthread_rwlock_tryrdlock(&m_lock) == 0;
}

bool CRWLock::try_wlock() {
    return pthread_rwlock_trywrlock(&m_lock) == 0;
}


////////////////////////////CAutoRWLock/////////////////////////////////////

/// @brief 构造函数
/// @param pLock 指向CRWLock对象的指针，表示要操作的读写锁
/// @param bRLock 布尔值用于指定是否进行读锁操作 ture进行读锁操作 false进行写锁操作
CAutoRWLock::CAutoRWLock(CRWLock* pLock, bool bRLock) {
    m_pLock = pLock;
    if (NULL != m_pLock) {
        // 据传入的参数bRLock来选择执行读锁操作还是写锁操作（获取锁和上锁实际上是指同一个操作）
        // 如果写锁当前可用，则获取成功，线程可以继续执行
        // 如果写锁当前被其他线程持有，则获取失败，线程会被阻塞，直到写锁可用为止
        if (bRLock) m_pLock->rlock();//获取读锁
        else m_pLock->wlock();//获取写锁
    }
}

CAutoRWLock::~CAutoRWLock() {
    if (NULL != m_pLock) m_pLock->unlock();
}

#endif
