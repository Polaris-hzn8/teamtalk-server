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

#include "common/util.h"
#include "common/Lock.h"
#include "common/ostype.h"

enum {
	SOCKET_READ		= 0x1,
	SOCKET_WRITE	= 0x2,
	SOCKET_EXCEP	= 0x4,
	SOCKET_ALL		= 0x7
};

// 套接字事件处理分发
class CEventDispatch
{
public:
	virtual ~CEventDispatch();

	void AddTimer(callback_t callback, void* user_data, uint64_t interval);
	void RemoveTimer(callback_t callback, void* user_data);
    void AddLoop(callback_t callback, void* user_data);

	bool isRunning() {return running;}

	void AddEvent(SOCKET fd, uint8_t socket_event);
	void RemoveEvent(SOCKET fd, uint8_t socket_event);

	void StartDispatch(uint32_t wait_timeout = 100);
    void StopDispatch();

	static CEventDispatch* Instance();
protected:
	CEventDispatch();

private:
	void _CheckTimer();
    void _CheckLoop();

	typedef struct {
		callback_t	callback;
		void*		user_data;
		uint64_t	interval;
		uint64_t	next_tick;
	} TimerItem;

private:
#ifdef _WIN32
	fd_set	m_read_set;
	fd_set	m_write_set;
	fd_set	m_excep_set;
#elif __APPLE__
	int 	m_kqfd;
#else
	int		m_epfd;
#endif
	CLock					m_lock;
	std::list<TimerItem*>	m_timer_list;
	std::list<TimerItem*>	m_loop_list;
    
    bool running;
};

#endif
