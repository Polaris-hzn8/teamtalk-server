/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: Thread.cpp
 Update Time: Tue 13 Jun 2023 09:01:58 CST
 brief:
*/

#include "Thread.h"

////////////////////////////////////////CThread//////////////////////////////////////
CThread::CThread()
{
    m_thread_id = 0;
}

CThread::~CThread()
{

}

#ifdef _WIN32
DWORD WINAPI CThread::StartRoutine(LPVOID arg)
#else
void* CThread::StartRoutine(void* arg)
#endif
{
    CThread* pThread = (CThread*)arg;
    pThread->OnThreadRun();
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

void CThread::StartThread()
{
#ifdef _WIN32
    (void)CreateThread(NULL, 0, StartRoutine, this, 0, &m_thread_id);
#else
    (void)pthread_create(&m_thread_id, NULL, StartRoutine, this);
#endif
}

////////////////////////////////////////CEventThread//////////////////////////////////////
CEventThread::CEventThread()
{
    m_bRunning = false;
}

CEventThread::~CEventThread()
{
    StopThread();
}

void CEventThread::StartThread()
{
    m_bRunning = true;
    CThread::StartThread();
}

void CEventThread::StopThread()
{
    m_bRunning = false;
}

void CEventThread::OnThreadRun()
{
    while (m_bRunning)
        OnThreadTick();
}

////////////////////////////////////////CThreadNotify//////////////////////////////////////
CThreadNotify::CThreadNotify()
{
    pthread_mutexattr_init(&m_mutexattr);
    pthread_mutexattr_settype(&m_mutexattr, PTHREAD_MUTEX_RECURSIVE);//递归锁
    pthread_mutex_init(&m_mutex, &m_mutexattr);
    pthread_cond_init(&m_cond, NULL);
}

CThreadNotify::~CThreadNotify()
{
    pthread_mutexattr_destroy(&m_mutexattr);
    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_cond);
}

