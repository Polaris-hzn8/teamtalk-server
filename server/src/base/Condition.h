/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: Condition.h
 Update Time: Sun 11 Jun 2023 22:18:04 CST
 brief: 
*/

#ifndef __CONDITION_H__
#define __CONDITION_H__

#include <pthread.h>
#include "Lock.h"

/**
 * 线程同步条件变量的抽象
*/
class CCondition {
public:
    //将条件变量与特定的锁相关联
    CCondition(CLock* pLock);
    //清理与条件变量相关的任何资源
    ~CCondition();
    //阻塞调用线程，直到条件变量被发信号
    //在调用该函数时，必须持有关联的锁
    //它将释放锁，并等待另一个线程发出的信号，然后重新获取锁并返回
    void wait();
    //类似于wait() 但它包括一个以毫秒（nWaitTime）为单位的超时值
    //如果在指定的时间内收到信号，它将返回true 否则，如果超时时间到期而未收到信号，它将返回false
    bool waitTime(uint64_t nWaitTime);
    //通知一个等待的线程条件已发生 它唤醒一个等待的线程（如果有），使其可以继续执行
    void notify();
    //通知所有等待的线程条件已发生 它唤醒所有等待的线程，使它们可以继续执行
    void notifyAll();
private:
    CLock* m_pLock;//指向关联的CLock对象的指针
    pthread_cond_t m_cond;//实际的条件变量 用于线程同步
};

#endif
