/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: ThreadPool.cpp
 Update Time: Tue 13 Jun 2023 10:15:50 CST
 brief:
*/

#include "ThreadPool.h"
#include "util.h"
#include <stdlib.h>

///////////////////////////////////////////CWorkerThread//////////////////////////////////////////
CWorkerThread::CWorkerThread() {
    m_task_cnt = 0;
}

CWorkerThread::~CWorkerThread() {}

/**
 * 将工作线程的执行入口函数StartRoutine 与工作线程对象pThread进行关联
 * 在启动线程时，会调用这个静态成员函数作为线程的入口点，然后在函数中通过工作线程对象调用实际的任务处理逻辑
*/
void* CWorkerThread::StartRoutine(void* arg) {
	//将传入的参数arg转换为CWorkerThread*类型，将其赋值给指针变量pThread
    CWorkerThread* pThread = (CWorkerThread*)arg;
	//调用Execute()函数，该函数用于执行工作线程的任务处理逻辑
	//这个函数是CWorkerThread类的成员函数，会被具体的工作线程对象调用
    pThread->Execute();
    return NULL;
}

void CWorkerThread::Start() {
	//调用pthread_create()函数创建一个新的线程 并将线程标识符存储在成员变量m_thread_id中
	/**
	 * thread_create()函数接受四个参数
	 * @param &m_thread_id 指向线程标识符的指针，用于存储创建的线程的标识符
	 * @param NULL 用于设置默认的线程属性
	 * @param StartRoutine 线程的入口点函数，即静态成员函数StartRoutine
	 * @param this 作为线程的参数 将当前对象的指针传递给线程入口点函数
	*/
    (void)pthread_create(&m_thread_id, NULL, StartRoutine, this);
}

/**
 * 用于实际执行工作线程的任务处理逻辑
 * 使用一个无限循环来表示工作线程的执行过程 循环的每次迭代代表一个任务的处理逻辑
*/
void CWorkerThread::Execute() {
    while (true) {
		// 1.调用m_thread_notify.Lock()来获取线程通知对象的互斥锁，以进行线程同步
        m_thread_notify.Lock();

		// 2.使用while循环检查任务队列m_task_list是否为空
		// 由于存在虚假唤醒的可能性，这里使用while循环而不是if条件语句（惊群效应）
		// 如果任务队列为空，则调用m_thread_notify.Wait()进入等待状态，直到有新任务被添加到队列中并发出通知信号。
        // put wait in while cause there can be spurious wake up (due to signal/ENITR)
        while (m_task_list.empty()) m_thread_notify.Wait();

		// 3.一旦有任务被添加到队列中，会从任务队列的前端取出一个任务对象，并将其从队列中移除
        CTask* pTask = m_task_list.front();
        m_task_list.pop_front();

		// 4.使用m_thread_notify.Unlock()释放线程通知对象的互斥锁，以便其他线程可以访问任务队列
        m_thread_notify.Unlock();

		// 5.调用任务对象的run()函数，执行具体的任务逻辑
        pTask->run();

		// 6.使用delete操作符释放任务对象的内存空间，避免内存泄漏。
        delete pTask;

		// 7.增加任务计数m_task_cnt的值，表示已执行的任务数量
        m_task_cnt++;
        // log("%d have the execute %d task\n", m_thread_idx, m_task_cnt);
    }
}

/**
 * 将任务添加到工作线程的任务队列中，以便工作线程能够按序处理任务
*/
void CWorkerThread::PushTask(CTask* pTask) {
	//调用m_thread_notify.Lock()获取线程通知对象的互斥锁，以进行线程同步
    m_thread_notify.Lock();
	//将任务对象指针pTask添加到任务队列m_task_list的末尾，使用push_back()函数实现
    m_task_list.push_back(pTask);
	//调用m_thread_notify.Signal()发送一个信号给正在等待的工作线程，通知有新任务可用
    m_thread_notify.Signal();
	//调用m_thread_notify.Unlock()释放线程通知对象的互斥锁，以便其他线程可以访问任务队列
    m_thread_notify.Unlock();
}


///////////////////////////////////////////CThreadPool//////////////////////////////////////////
CThreadPool::CThreadPool() {
    m_worker_size = 0;
    m_worker_list = NULL;
}

CThreadPool::~CThreadPool() {}

/**
 * 初始化线程池，创建指定数量的工作线程，并启动它们以准备处理任务
*/
int CThreadPool::Init(uint32_t worker_size) {
    m_worker_size = worker_size;
    m_worker_list = new CWorkerThread[m_worker_size];
    if (!m_worker_list) return 1;

	//使用循环遍历每个工作线程，设置其线程索引 SetThreadIdx(i) 和启动线程 Start()
    for (uint32_t i = 0; i < m_worker_size; i++) {
        m_worker_list[i].SetThreadIdx(i);
        m_worker_list[i].Start();
    }

    return 0;
}

void CThreadPool::Destory() {
    if (m_worker_list) delete[] m_worker_list;
}

/**
 * 向线程池中的工作线程添加任务
 * 选择一个随机的工作线程来添加任务
 * 这样可以平均分配任务负载，避免某个工作线程一直处理大量任务而其他工作线程处于空闲状态
 * 
 * 也可以根据具体的需求采用其他任务分配策略，例如选择任务最少的工作线程来添加任务
*/
void CThreadPool::AddTask(CTask* pTask) {
	//通过random() % m_worker_size生成一个随机数，范围在0到m_worker_size-1之间，用于选择一个随机的工作线程索引
    uint32_t thread_idx = random() % m_worker_size;
	//使用选定的线程索引thread_idx访问线程池中的对应工作线程对象m_worker_list[thread_idx]
	//调用选定的工作线程对象的PushTask(pTask)函数，将任务对象指针pTask添加到该工作线程的任务队列中
	//选定的工作线程将在适当的时候从任务队列中获取任务并执行
    m_worker_list[thread_idx].PushTask(pTask);
}



