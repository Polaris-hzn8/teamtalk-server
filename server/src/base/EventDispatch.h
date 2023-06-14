/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: EventDispatch.h
 Update Time: Wed 14 Jun 2023 15:25:05 CST
 brief: 事件调度器（CEventDispatch）用于处理事件的调度和触发
	A socket event dispatcher, features include: 
	1. portable: worked both on Windows, MAC OS X,  LINUX platform（多平台）
	2. a singleton pattern: only one instance of this class can exist（应用单例模式）
*/

#ifndef __EVENT_DISPATCH_H__
#define __EVENT_DISPATCH_H__

#include "ostype.h"
#include "util.h"

#include "Lock.h"

//用于事件类型的常量
enum {
	SOCKET_READ		= 0x1,
	SOCKET_WRITE	= 0x2,
	SOCKET_EXCEP	= 0x4,
	SOCKET_ALL		= 0x7
};


/**
 * CEventDispatch类 用于实现事件调度器的功能
 *  - 事件调度器（CEventDispatch）用于管理和调度事件和定时器
 *  - 可以添加套接字的读、写和异常事件，并通过回调函数通知上层逻辑
 *  - 此外还支持添加定时器和循环任务，以便在指定的时间间隔内执行相应的操作
 * 该类定义提供了一种机制，可以方便地使用事件调度器来处理套接字事件和定时任务，以 实现高效的事件驱动编程模型
*/
class CEventDispatch {
public:
	virtual ~CEventDispatch();

	//添加套接字事件
	void AddEvent(SOCKET fd, uint8_t socket_event);
	//删除套接字事件
	void RemoveEvent(SOCKET fd, uint8_t socket_event);
	//添加定时器
	void AddTimer(callback_t callback, void* user_data, uint64_t interval);
	//删除定时器
	void RemoveTimer(callback_t callback, void* user_data);
	//添加循环任务
    void AddLoop(callback_t callback, void* user_data);

	//启动事件调度器
	void StartDispatch(uint32_t wait_timeout = 100);
	//停止事件调度器
    void StopDispatch();
    //判断事件调度器是否正在运行
    bool isRunning() {return running;}

	//获取事件调度器实例
	static CEventDispatch* Instance();
protected:
	CEventDispatch();

private:
	//检查定时器
	void _CheckTimer();
	//检查循环任务
    void _CheckLoop();

	//定时器结构体 用于存储定时器的信息
	typedef struct {
		callback_t	callback;//回调函数
		void*		user_data;//用户数据
		uint64_t	interval;//时间间隔
		uint64_t	next_tick;//下一次触发的时间戳
	} TimerItem;

private:

#ifdef _WIN32
	//在Windows平台下，定义了三个 fd_set 类型的成员变量
	fd_set	m_read_set;//存储需要监视读事件的套接字集合
	fd_set	m_write_set;//存储需要监视写事件的套接字集合
	fd_set	m_excep_set;//存储需要监视异常事件的套接字集合
#elif __APPLE__
	//在苹果macOS平台定义了一个int类型的成员变量 m_kqfd
	int 	m_kqfd;//用于存储内核事件队列的文件描述符。
#else
	//在其他平台
	int		m_epfd;//用于存储epoll实例的文件描述符
#endif

	CLock			m_lock;//用于保护并发访问的互斥锁
	list<TimerItem*>	m_timer_list;//存储定时器项的链表
	list<TimerItem*>	m_loop_list;//存储循环任务项的链表

	static CEventDispatch* m_pEventDispatch;//静态指针，指向CEventDispatch类的唯一实例
    
    bool running;//事件调度器是否正在运行
};

#endif
