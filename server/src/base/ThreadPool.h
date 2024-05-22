/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ThreadPool.h
 Update Time: Tue 13 Jun 2023 10:15:17 CST
 brief: 
*/

#ifndef THREADPOOL_H_
#define THREADPOOL_H_
#include "ostype.h"
#include "Thread.h"
#include "Task.h"
#include <pthread.h>
#include <list>
using namespace std;

/**
 * CWorkerThread类是一个工作线程的实现
*/
class CWorkerThread {
public:
	//初始化工作线程对象
	CWorkerThread();
	//清理工作线程对象
	~CWorkerThread();

	//静态成员函数StartRoutine(void* arg)：作为线程的入口点函数，接受一个参数作为线程的参数
	static void* StartRoutine(void* arg);

	//启动工作线程
	void Start();
	//执行工作线程的任务处理逻辑
	void Execute();
	//将任务对象pTask推入任务队列
	void PushTask(CTask* pTask);
	//设置工作线程的索引
	void SetThreadIdx(uint32_t idx) { m_thread_idx = idx; }

private:
	uint32_t		m_thread_idx;//工作线程的索引
	uint32_t		m_task_cnt;//任务计数，用于跟踪任务的数量
	pthread_t		m_thread_id;//线程标识符
	CThreadNotify	m_thread_notify;//线程通知对象，用于线程同步
	list<CTask*>	m_task_list;//任务队列，存储待处理的任务对象
};

/**
 * CThreadPool类表示线程池，用于管理一组工作线程
*/
class CThreadPool {
public:
	//初始化线程池对象
	CThreadPool();
	//清理线程池对象
	virtual ~CThreadPool();

	//初始化线程池，创建指定数量的工作线程
	int Init(uint32_t worker_size);
	//向线程池添加任务对象
	void AddTask(CTask* pTask);
	//销毁线程池，停止所有工作线程
	void Destory();
private:
	uint32_t 		m_worker_size;//线程池中的工作线程数量
	CWorkerThread* 	m_worker_list;//工作线程对象数组
};

#endif
