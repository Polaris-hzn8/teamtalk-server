/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: Thread.cpp
 Update Time: Tue 13 Jun 2023 09:01:58 CST
 brief:
*/

#include "Thread.h"

////////////////////////////////////////CThread//////////////////////////////////////
CThread::CThread() {
    m_thread_id = 0;
}

CThread::~CThread() { }

/**
 * StartRoutine 作为线程的入口点
 * 根据编译平台的不同，函数的返回类型和参数类型可能会有所不同
 * 
 * 在Windows平台 StartRoutine的返回类型为DWORD 参数类型为LPVOID（相当于void*）
 * 在非Windows平台，返回类型为void*，参数类型为void*
*/
#ifdef _WIN32
DWORD WINAPI CThread::StartRoutine(LPVOID arg)
#else
void* CThread::StartRoutine(void* arg)
#endif
{
	//首先将传递给线程的参数转换为CThread*类型，并将其赋值给pThread变量
    CThread* pThread = (CThread*)arg;
	//调用OnThreadRun()函数
	//OnThreadRun是一个虚函数由派生类实现 在派生类中可以重写OnThreadRun()函数来定义线程的具体行为
    pThread->OnThreadRun();

#ifdef _WIN32
    return 0;//在Windows平台，返回值为0
#else
    return NULL;//在非Windows平台，返回值为NULL
#endif
}


/**
 * StartThread 用于启动线程
 * 根据编译平台的不同，函数使用不同的线程创建方法
*/
void CThread::StartThread() {
#ifdef _WIN32
	//在Windows平台使用CreateThread函数来创建线程
	//函数参数包括 线程属性NULL、堆栈大小0、线程入口点函数StartRoutine 和传递给线程的参数this 创建线程时使用默认的标志0 线程的标识符m_thread_id
    (void)CreateThread(NULL, 0, StartRoutine, this, 0, &m_thread_id);
#else
	//在非Windows平台使用pthread_create函数来创建线程
	//函数参数包括 线程标识符m_thread_id、线程属性NULL、线程入口点函数StartRoutine 和传递给线程的参数 this
    (void)pthread_create(&m_thread_id, NULL, StartRoutine, this);
#endif
}

////////////////////////////////////////CEventThread//////////////////////////////////////
CEventThread::CEventThread() {
    m_bRunning = false;
}

CEventThread::~CEventThread() {
    StopThread();
}

void CEventThread::StartThread() {
    m_bRunning = true;
    CThread::StartThread();
}

void CEventThread::StopThread() {
    m_bRunning = false;
}

/**
 * OnThreadRun 重写了基类CThread的OnThreadRun()函数
 * 在函数中使用一个循环来不断调用OnThreadTick()函数，直到m_bRunning标志为 false 为止
 * 
 * 这样的设计使得线程能够在启动后持续执行指定的操作
 * 每次循环中，调用OnThreadTick函数，该函数是一个纯虚函数 派生类必须实现它以定义每个周期内的操作
*/
void CEventThread::OnThreadRun() {
    while (m_bRunning) OnThreadTick();
}


////////////////////////////////////////CThreadNotify//////////////////////////////////////
/**
 * CThreadNotify构造函数
*/
CThreadNotify::CThreadNotify() {
	//初始化互斥锁属性对象
    pthread_mutexattr_init(&m_mutexattr);
	//设置互斥锁的类型为PTHREAD_MUTEX_RECURSIVE，表示这是一个递归锁允许同一线程多次对锁进行加锁
    pthread_mutexattr_settype(&m_mutexattr, PTHREAD_MUTEX_RECURSIVE);
	//初始化互斥锁 并将属性对象m_mutexattr传递给它，以指定互斥锁的属性。
    pthread_mutex_init(&m_mutex, &m_mutexattr);
	//初始化条件变量 m_cond 并使用默认的属性
    pthread_cond_init(&m_cond, NULL);
}

/**
 * ~CThreadNotify析构函数
*/
CThreadNotify::~CThreadNotify() {
	//销毁互斥锁属性对象 以释放相关资源
    pthread_mutexattr_destroy(&m_mutexattr);
	//销毁互斥锁m_mutex，以释放互斥锁的资源
    pthread_mutex_destroy(&m_mutex);
	//销毁条件变量m_cond，以释放条件变量的资源
    pthread_cond_destroy(&m_cond);
}



