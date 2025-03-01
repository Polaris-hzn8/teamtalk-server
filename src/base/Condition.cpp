/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: Condition.cpp
 Update Time: Sun 11 Jun 2023 22:18:17 CST
 brief: 
*/

#include <assert.h>
#include "Condition.h"

CCondition::CCondition(CLock* pLock) : m_pLock(pLock) {
    if (!pLock) assert(false);
    //初始化条件变量
    pthread_cond_init(&m_cond, NULL);
}

CCondition::~CCondition() {
    //销毁条件变量
    pthread_cond_destroy(&m_cond);
}

/**
 * 等待条件变量的发生
 * 当一个线程调用wait()函数时，它会首先释放关联的锁（通过调用m_pLock->getMutex()获取锁）
 * 然后线程进入等待状态，直到其他线程发出条件信号（通过调用notify()或notifyAll()函数）
*/
void CCondition::wait() {
    //wait函数调用pthread_cond_wait来等待条件变量
    //释放关联的锁CLock 并等待直到其他线程发出条件信号
    pthread_cond_wait(&m_cond, &m_pLock->getMutex());
}

//带有指定的超时时间等待条件变量
bool CCondition::waitTime(uint64_t nWaitTime) {
    //根据输入的nWaitTime计算超时值 转换为struct timespec类型
    uint64_t nTime = nWaitTime * 1000000;
    struct timespec sTime;
    uint64_t nSec = nTime / (1000000000);
    uint64_t nNsec = nTime % (1000000000);
    sTime.tv_sec = time(NULL) + (uint32_t)nSec;
    sTime.tv_nsec = (uint32_t)nNsec;

    //调用pthread_cond_timedwait
    //如果超时时间到期（返回ETIMEDOUT）则返回false 否则返回true
    if (ETIMEDOUT == pthread_cond_timedwait(&m_cond, &m_pLock->getMutex(), &sTime)) return false;
    return true;
}

void CCondition::notify() {
    //notify()函数通过pthread_cond_signal发出条件变量的信号，唤醒一个等待的线程
    pthread_cond_signal(&m_cond);
}

void CCondition::notifyAll() {
    //notifyAll()函数通过pthread_cond_broadcast发出条件变量的信号，唤醒所有等待的线程
    pthread_cond_broadcast(&m_cond);
}

