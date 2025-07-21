/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ThreadPool.h
 Update Time: Tue 13 Jun 2023 10:15:17 CST
 brief: 
*/

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <list>
#include <pthread.h>
#include "Task.h"
#include "ostype.h"
#include "Thread.h"
using namespace std;

class CWorkerThread
{
public:
	CWorkerThread();
	~CWorkerThread()

	static void* StartRoutine(void* arg);

	void Start();
	void Execute();
	void PushTask(CTask* pTask);
	void SetThreadIdx(uint32_t idx) { m_thread_idx = idx; }
private:
	uint32_t		m_thread_idx;		//线程idx
	uint32_t		m_task_cnt;			//任务数量
	pthread_t		m_thread_id;		//线程id
	CThreadNotify	m_thread_notify;	//线程通知对象，用于线程同步
	list<CTask*>	m_task_list;		//线程任务队列
};

class CThreadPool
{
public:
	CThreadPool();
	virtual ~CThreadPool();

	int Init(uint32_t worker_size);
	void AddTask(CTask* pTask);
	void Destory();
private:
	uint32_t 		m_worker_size;		//线程池大小
	CWorkerThread* 	m_worker_list;		//线程池数组
};

#endif
