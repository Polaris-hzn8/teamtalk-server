/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: Thread.h
 Update Time: Tue 13 Jun 2023 09:01:51 CST
 brief:
*/

#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>

// CThread线程类封装
// 通过实现具体的OnThreadRun()函数来定义线程行为
class CThread
{
public:
    CThread();
    virtual ~CThread();

#ifdef _WIN32
    static DWORD WINAPI StartRoutine(LPVOID lpParameter);
#else
    static void* StartRoutine(void* arg);
#endif

    virtual void StartThread(void);
    virtual void OnThreadRun(void) = 0;

protected:
#ifdef _WIN32
    DWORD       m_thread_id;
#else
    pthread_t   m_thread_id;//线程的标识符
#endif
};

// 针对CThread的扩展
// 通过实现具体的OnThreadTick()函数来定义线程行为
class CEventThread : public CThread
{
public:
    CEventThread();
    virtual ~CEventThread();

    virtual void OnThreadTick(void) = 0;//线程任务逻辑
    virtual void OnThreadRun(void);
    virtual void StartThread();
    virtual void StopThread();

    bool IsRunning() { return m_bRunning; }
private:
    bool m_bRunning;
};

// 线程间的同步
class CThreadNotify
{
public:
    CThreadNotify();
    ~CThreadNotify();

    void Lock() { pthread_mutex_lock(&m_mutex); }
    void Unlock() { pthread_mutex_unlock(&m_mutex); }
    void Wait() { pthread_cond_wait(&m_cond, &m_mutex); }
    void Signal() { pthread_cond_signal(&m_cond); }

private:
    pthread_mutex_t         m_mutex;
    pthread_mutexattr_t     m_mutexattr;
    pthread_cond_t          m_cond;
};

#endif
