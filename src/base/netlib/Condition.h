/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: Condition.h
 Update Time: Sun 11 Jun 2023 22:18:04 CST
 brief: 
*/

#ifndef __CONDITION_H__
#define __CONDITION_H__

#include "Lock.h"
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <time.h>
#include <errno.h>
#endif

class CCondition
{
public:
    CCondition(CLock* pLock);
    ~CCondition();

    void wait();
    bool waitTime(uint64_t nWaitTime);
    void notify();
    void notifyAll();
private:
    CLock*          m_pLock;
#ifdef _WIN32
    CONDITION_VARIABLE m_cond;
#else
    pthread_cond_t m_cond;
#endif
};

#endif
