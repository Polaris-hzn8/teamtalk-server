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
#include <pthread.h>

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
    pthread_cond_t  m_cond;
};

#endif
