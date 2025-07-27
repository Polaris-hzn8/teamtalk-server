/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ThreadPool.cpp
 Update Time: Tue 13 Jun 2023 10:15:50 CST
 brief:
*/

#include <stdlib.h>
#include "ThreadPool.h"
#include "common/util.h"

////////////////////////////////////////////////////////////////////////////////
// CWorkerThread
CWorkerThread::CWorkerThread()
{
    m_task_cnt = 0;
}

CWorkerThread::~CWorkerThread()
{

}

void* CWorkerThread::StartRoutine(void* arg)
{
    CWorkerThread* pThread = (CWorkerThread*)arg;
    pThread->Execute();
    return NULL;
}

void CWorkerThread::Start()
{
    (void)pthread_create(&m_thread_id, NULL, StartRoutine, this);
}

void CWorkerThread::Execute()
{
    while (true) {
        m_thread_notify.Lock();
        while (m_task_list.empty())
            m_thread_notify.Wait();

        CTask* pTask = m_task_list.front();
        m_task_list.pop_front();
        m_thread_notify.Unlock();

        pTask->run();
        delete pTask;
        m_task_cnt++;
    }
}

void CWorkerThread::PushTask(CTask* pTask)
{
    m_thread_notify.Lock();
    m_task_list.push_back(pTask);
    m_thread_notify.Signal();
    m_thread_notify.Unlock();
}

////////////////////////////////////////////////////////////////////////////////
// CThreadPool
CThreadPool::CThreadPool()
{
    m_worker_size = 0;
    m_worker_list = NULL;
}

CThreadPool::~CThreadPool()
{

}

int CThreadPool::Init(uint32_t worker_size)
{
    m_worker_size = worker_size;
    m_worker_list = new CWorkerThread[m_worker_size];
    if (!m_worker_list)
        return 1;

    // 遍历所有工作线程 设置线程索引并启动执行
    for (uint32_t i = 0; i < m_worker_size; i++) {
        m_worker_list[i].SetThreadIdx(i);
        m_worker_list[i].Start();
    }

    return 0;
}

// 随机选择工作线程来执行任务
void CThreadPool::AddTask(CTask* pTask)
{
    uint32_t thread_idx = random() % m_worker_size;
    m_worker_list[thread_idx].PushTask(pTask);
}

void CThreadPool::Destory()
{
    if (m_worker_list)
        delete[] m_worker_list;
}
