/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: Condition.cpp
 Update Time: Sun 11 Jun 2023 22:18:17 CST
 brief: 
*/

#include <chrono>
#include <assert.h>
#include "Condition.h"

CCondition::CCondition(CLock* pLock) : m_pLock(pLock)
{
    if (!pLock)
        assert(false);
#ifdef _WIN32
    InitializeConditionVariable(&m_cond);
#else
    pthread_cond_init(&m_cond, NULL);
#endif
}

CCondition::~CCondition()
{
#ifndef _WIN32
    pthread_cond_destroy(&m_cond);
#endif
}

void CCondition::wait()
{
#ifdef _WIN32
    SleepConditionVariableCS(&m_cond, &m_pLock->getMutex(), INFINITE);
#else
    pthread_cond_wait(&m_cond, &m_pLock->getMutex());
#endif
}

// 毫秒
bool CCondition::waitTime(uint64_t nWaitTime)
{
#ifdef _WIN32
    // Windows 单位是毫秒
    BOOL ret = SleepConditionVariableCS(&m_cond, &m_pLock->getMutex(), (DWORD)nWaitTime);
    return (ret != 0);
#else
    using namespace std::chrono;
    time_point<system_clock> now = system_clock::now();
    time_point<system_clock> timeout_time = now + milliseconds(nWaitTime);

    // 转换为 timespec
    time_point<system_clock, seconds> sec = time_point_cast<seconds>(timeout_time);

    struct timespec ts;
    ts.tv_sec = sec.time_since_epoch().count();
    ts.tv_nsec = duration_cast<nanoseconds>(timeout_time - sec).count();

    int rc = pthread_cond_timedwait(&m_cond, &m_pLock->getMutex(), &ts);
    return rc != ETIMEDOUT;
#endif
}

void CCondition::notify()
{
#ifdef _WIN32
    WakeConditionVariable(&m_cond);
#else
    pthread_cond_signal(&m_cond);
#endif
}

void CCondition::notifyAll()
{
#ifdef _WIN32
    WakeAllConditionVariable(&m_cond);
#else
    pthread_cond_broadcast(&m_cond);
#endif
}
