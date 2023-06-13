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

/**
 * CThread的类是一个线程的抽象
 * CThread类提供了一个基本的线程封装，用于创建和管理线程
 * 派生类可以通过实现OnThreadRun()函数来定义线程的具体行为
*/
class CThread {
public:
    CThread();
    virtual ~CThread();

/**
 * 静态函数StartRoutine用于线程的启动
 * 在Windows平台下，它返回DWORD类型
 * 在非Windows平台下，它返回void*类型
 * 函数是用于启动线程的入口点，实际执行的是CThread对象的OnThreadRun()函数
*/
#ifdef _WIN32
    static DWORD WINAPI StartRoutine(LPVOID lpParameter);
#else
    static void* StartRoutine(void* arg);
#endif

	//启动线程StartThread 会创建一个新线程，并将线程的入口点设置为StartRoutine函数
    virtual void StartThread(void);
	//抽象函数，派生类需要实现该函数 以定义线程的具体执行逻辑
    virtual void OnThreadRun(void) = 0;

protected:
#ifdef _WIN32
    DWORD m_thread_id;
#else
    pthread_t m_thread_id;//线程的标识符
#endif
};

/**
 * CEventThread类扩展了基类CThread 并为派生类提供了一个框架 使其可以实现周期性的操作
 * 派生类需要实现OnThreadTick()函数来定义每个周期内的操作
*/
class CEventThread : public CThread {
public:
    CEventThread();
    virtual ~CEventThread();

	//派生类需要实现该函数以定义线程的周期性操作
    virtual void OnThreadTick(void) = 0;
	//重写了基类CThread的OnThreadRun函数
	//在派生类中，OnThreadRun()函数会在线程启动时被调用，并在一个循环中不断调用OnThreadTick()函数
    virtual void OnThreadRun(void);
	//重写了基类CThread的StartThread()函数 用于启动线程并将m_bRunning标志设置为true
    virtual void StartThread();
	//停止线程的执行，并将m_bRunning标志设置为false
    virtual void StopThread();
	//检查线程的运行状态
    bool IsRunning() { return m_bRunning; }
private:
	//标识线程是否正在运行
    bool m_bRunning;
};

/**
 * CThreadNotify的类，用于线程间的通知和同步
 * CThreadNotify类封装了互斥锁和条件变量的操作，提供了简单的接口用于线程的同步和通知
 * 	- 通过调用Lock()和Unlock()来控制对共享资源的访问
 * 	- Wait()等待条件变量的发生
 * 	- Signal()发送条件信号通知等待的线程
*/
class CThreadNotify {
public:
	//初始化类的成员变量和互斥锁
    CThreadNotify();
	//销毁互斥锁和条件变量
    ~CThreadNotify();

	//获取互斥锁 阻塞线程直到成功获取锁
    void Lock() { pthread_mutex_lock(&m_mutex); }
	//释放互斥锁 调用pthread_mutex_unlock来释放互斥锁
    void Unlock() { pthread_mutex_unlock(&m_mutex); }
	//等待条件变量 调用pthread_cond_wait，使线程进入等待状态，直到条件变量被其他线程发出信号
    void Wait() { pthread_cond_wait(&m_cond, &m_mutex); }
	//发送条件信号 调用pthread_cond_signal来发送条件变量的信号，唤醒一个等待的线程
    void Signal() { pthread_cond_signal(&m_cond); }

private:
    pthread_mutex_t m_mutex;//互斥锁 实现对共享资源的互斥访问
    pthread_mutexattr_t m_mutexattr;//互斥锁的属性，使用默认的属性
    pthread_cond_t m_cond;//条件变量，用于线程间的同步和通知
};

#endif
