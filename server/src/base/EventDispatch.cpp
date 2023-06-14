/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: EventDispatch.cpp
 Update Time: Wed 14 Jun 2023 15:26:17 CST
 brief: 事件调度器（CEventDispatch）用于处理事件的调度和触发
	根据不同操作系统的特性，实现了对应的事件调度机制
	包括Windows平台的select，苹果平台的kqueue 和其他平台的epoll
*/

#include "EventDispatch.h"
#include "BaseSocket.h"

#define MIN_TIMER_DURATION 100 // 100 miliseconds

CEventDispatch* CEventDispatch::m_pEventDispatch = NULL;

CEventDispatch::CEventDispatch() {
    running = false;

#ifdef _WIN32
	//初始化了不同操作系统平台下的相关数据结构
	//初始化fd文件描述符集合
    FD_ZERO(&m_read_set);
    FD_ZERO(&m_write_set);
    FD_ZERO(&m_excep_set);
#elif __APPLE__
    m_kqfd = kqueue();
    if (m_kqfd == -1) log_error("kqueue failed");
#else
	//创建一个epoll实例 并设置最多可以监听1024个文件描述符
    m_epfd = epoll_create(1024);
    if (m_epfd == -1) log_error("epoll_create failed");
#endif

}

CEventDispatch::~CEventDispatch() {
#ifdef _WIN32
	//销毁不同操作系统平台下的相关数据结构
#elif __APPLE__
    close(m_kqfd);
#else
    close(m_epfd);
#endif
}

/**
 * 向事件调度器中添加一个定时器（定时回调）
 * 定时器项包含了定时器的相关信息，包括回调函数、间隔时间和下一次触发时间
 * 在后续的事件调度过程中，事件调度器会根据定时器项的信息来触发定时器的回调函数
 * 如果已经存在相同回调函数和用户数据的定时器项，会更新其间隔时间和下一次触发时间，以实现定时器的调整功能
*/
/// @brief 向事件调度器中添加一个定时器
/// @param callback 定时器的回调函数
/// @param user_data 传递给回调函数的用户数据
/// @param interval 定时器的间隔时间
void CEventDispatch::AddTimer(callback_t callback, void* user_data, uint64_t interval) {
    list<TimerItem*>::iterator it;
	// 1.遍历存储定时器的链表 m_timer_list，查找是否已经存在具有相同回调函数和用户数据的定时器项
    for (it = m_timer_list.begin(); it != m_timer_list.end(); it++) {
        TimerItem* pItem = *it;
        if (pItem->callback == callback && pItem->user_data == user_data) {
			// 2.如果找到了相同的定时器项，更新该定时器项的间隔时间和下一次触发时间
            pItem->interval = interval;
            pItem->next_tick = get_tick_count() + interval;
            return;
        }
    }

	// 3.如果没有找到相同的定时器项，创建一个新的定时器项，并设置回调函数、用户数据、间隔时间和下一次触发时间
    TimerItem* pItem = new TimerItem;
    pItem->callback = callback;
    pItem->user_data = user_data;
    pItem->interval = interval;
    pItem->next_tick = get_tick_count() + interval;

	// 4.将新创建的定时器项添加到m_timer_list链表中
    m_timer_list.push_back(pItem);

}


/**
 * 从事件调度器中移除定时器
 * 在后续的事件调度过程中，被移除的定时器项将不再触发其回调函数
 * 通过比较回调函数和用户数据来确定要移除的定时器项。
*/
/// @brief 从事件调度器中移除定时器
/// @param callback 要移除的定时器的回调函数
/// @param user_data 要移除的定时器的用户数据
void CEventDispatch::RemoveTimer(callback_t callback, void* user_data) {
    list<TimerItem*>::iterator it;
	// 1.遍历存储定时器的链表m_timer_list，查找具有相同回调函数和用户数据的定时器项
    for (it = m_timer_list.begin(); it != m_timer_list.end(); it++) {
        TimerItem* pItem = *it;
        if (pItem->callback == callback && pItem->user_data == user_data) {
			// 2.如果找到了相同的定时器项，通过迭代器 it 删除该定时器项，并释放相应的内存资源
            m_timer_list.erase(it);
            delete pItem;
            return;
        }
    }
}


//用于检查和触发定时器事件
void CEventDispatch::_CheckTimer() {
	// 1.获取当前系统时钟curr_tick，通常使用get_tick_count函数获取
    uint64_t curr_tick = get_tick_count();

	// 2.遍历定时器列表 m_timer_list，其中存储了所有的定时器项
    list<TimerItem*>::iterator it;
    for (it = m_timer_list.begin(); it != m_timer_list.end();) {
		// 2-1.对于每个定时器项 pItem 进行判断
        TimerItem* pItem = *it;
        it++; //it指针可能会在回调时被调用 应该首先++
		// 2-2.如果当前时钟 curr_tick 大于等于下一次触发时间 表示定时器触发了
        if (curr_tick >= pItem->next_tick) {
			//更新下一次触发时间pItem->next_tick为 当前时钟加上定时器间隔时间pItem->interval，用于下一次触发
            pItem->next_tick += pItem->interval;
			//调用定时器回调函数 pItem->callback，传递相应的参数，通常是触发定时器事件
            pItem->callback(pItem->user_data, NETLIB_MSG_TIMER, 0, NULL);
        }
    }
}


/**
 * 用于向循环列表中添加循环事件
 * 循环事件会以一定的时间间隔反复触发，直到被移除
*/
void CEventDispatch::AddLoop(callback_t callback, void* user_data) {
    TimerItem* pItem = new TimerItem;//创建一个新的 TimerItem 对象 pItem，用于存储循环事件的信息
    pItem->callback = callback;//设置 pItem 的回调函数
    pItem->user_data = user_data;//设置 pItem 的用户数据
    m_loop_list.push_back(pItem);//将 pItem 添加到循环列表 m_loop_list 的末尾，以便后续的循环调度
}

/**
 * 该函数的作用是检查并触发事件调度器中的循环事件
 * 调用每个循环事件的回调函数，用于执行循环事件的逻辑
*/
void CEventDispatch::_CheckLoop() {
	//使用迭代器遍历循环列表 m_loop_list 中的每个循环事件
    for (list<TimerItem*>::iterator it = m_loop_list.begin(); it != m_loop_list.end(); it++) {
		//获取当前循环事件的指针 pItem
        TimerItem* pItem = *it;
		//调用循环事件的回调函数 pItem->callback，并传递相应的参数
		//NETLIB_MSG_LOOP 表示该事件是循环事件 0 表示事件句柄为0 NULL 表示没有附加数据
        pItem->callback(pItem->user_data, NETLIB_MSG_LOOP, 0, NULL);
    }
}

//静态成员函数 用于获取 CEventDispatch 类的单例实例
CEventDispatch* CEventDispatch::Instance() {
	//首先检查静态成员变量m_pEventDispatch是否为NULL 即是否已经创建了单例实例
	//如果没有则创建一个新的 CEventDispatch 对象并将其赋值给 m_pEventDispatch
    if (m_pEventDispatch == NULL) m_pEventDispatch = new CEventDispatch();
	//返回 m_pEventDispatch，即单例实例的指针
    return m_pEventDispatch;
}

#ifdef _WIN32

void CEventDispatch::AddEvent(SOCKET fd, uint8_t socket_event) {
    CAutoLock func_lock(&m_lock);

    if ((socket_event & SOCKET_READ) != 0) FD_SET(fd, &m_read_set);
    if ((socket_event & SOCKET_WRITE) != 0) FD_SET(fd, &m_write_set);
    if ((socket_event & SOCKET_EXCEP) != 0) FD_SET(fd, &m_excep_set);
}

void CEventDispatch::RemoveEvent(SOCKET fd, uint8_t socket_event) {
    CAutoLock func_lock(&m_lock);

    if ((socket_event & SOCKET_READ) != 0) FD_CLR(fd, &m_read_set);

    if ((socket_event & SOCKET_WRITE) != 0) FD_CLR(fd, &m_write_set);

    if ((socket_event & SOCKET_EXCEP) != 0) FD_CLR(fd, &m_excep_set);
}

void CEventDispatch::StartDispatch(uint32_t wait_timeout) {
    fd_set read_set, write_set, excep_set;
    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = wait_timeout * 1000; // 10 millisecond

    if (running) return;
    running = true;

    while (running) {
        _CheckTimer();
        _CheckLoop();

        if (!m_read_set.fd_count && !m_write_set.fd_count && !m_excep_set.fd_count) {
            Sleep(MIN_TIMER_DURATION);
            continue;
        }

        m_lock.lock();
        memcpy(&read_set, &m_read_set, sizeof(fd_set));
        memcpy(&write_set, &m_write_set, sizeof(fd_set));
        memcpy(&excep_set, &m_excep_set, sizeof(fd_set));
        m_lock.unlock();

        int nfds = select(0, &read_set, &write_set, &excep_set, &timeout);

        if (nfds == SOCKET_ERROR) {
            log_error("select failed, error code: %d", GetLastError());
            Sleep(MIN_TIMER_DURATION);
            continue; // select again
        }

        if (nfds == 0) continue;

        for (u_int i = 0; i < read_set.fd_count; i++) {
            // log("select return read count=%d\n", read_set.fd_count);
            SOCKET fd = read_set.fd_array[i];
            CBaseSocket* pSocket = FindBaseSocket((net_handle_t)fd);
            if (pSocket) {
                pSocket->OnRead();
                pSocket->ReleaseRef();
            }
        }

        for (u_int i = 0; i < write_set.fd_count; i++) {
            // log("select return write count=%d\n", write_set.fd_count);
            SOCKET fd = write_set.fd_array[i];
            CBaseSocket* pSocket = FindBaseSocket((net_handle_t)fd);
            if (pSocket) {
                pSocket->OnWrite();
                pSocket->ReleaseRef();
            }
        }

        for (u_int i = 0; i < excep_set.fd_count; i++) {
            // log("select return exception count=%d\n", excep_set.fd_count);
            SOCKET fd = excep_set.fd_array[i];
            CBaseSocket* pSocket = FindBaseSocket((net_handle_t)fd);
            if (pSocket) {
                pSocket->OnClose();
                pSocket->ReleaseRef();
            }
        }
    }
}

void CEventDispatch::StopDispatch() {
    running = false;
}

#elif __APPLE__

void CEventDispatch::AddEvent(SOCKET fd, uint8_t socket_event) {
    struct kevent ke;

    if ((socket_event & SOCKET_READ) != 0) {
        EV_SET(&ke, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
        kevent(m_kqfd, &ke, 1, NULL, 0, NULL);
    }

    if ((socket_event & SOCKET_WRITE) != 0) {
        EV_SET(&ke, fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
        kevent(m_kqfd, &ke, 1, NULL, 0, NULL);
    }
}

void CEventDispatch::RemoveEvent(SOCKET fd, uint8_t socket_event) {
    struct kevent ke;

    if ((socket_event & SOCKET_READ) != 0) {
        EV_SET(&ke, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
        kevent(m_kqfd, &ke, 1, NULL, 0, NULL);
    }

    if ((socket_event & SOCKET_WRITE) != 0) {
        EV_SET(&ke, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
        kevent(m_kqfd, &ke, 1, NULL, 0, NULL);
    }
}

void CEventDispatch::StartDispatch(uint32_t wait_timeout) {
    struct kevent events[1024];
    int nfds = 0;
    struct timespec timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = wait_timeout * 1000000;

    if (running) return;
    running = true;

    while (running) {
        nfds = kevent(m_kqfd, NULL, 0, events, 1024, &timeout);

        for (int i = 0; i < nfds; i++) {
            int ev_fd = events[i].ident;
            CBaseSocket* pSocket = FindBaseSocket(ev_fd);
            if (!pSocket) continue;

            if (events[i].filter == EVFILT_READ) {
                // log("OnRead, socket=%d\n", ev_fd);
                pSocket->OnRead();
            }

            if (events[i].filter == EVFILT_WRITE) {
                // log("OnWrite, socket=%d\n", ev_fd);
                pSocket->OnWrite();
            }

            pSocket->ReleaseRef();
        }

        _CheckTimer();
        _CheckLoop();
    }
}

void CEventDispatch::StopDispatch() {
    running = false;
}

#else


/// @brief 向事件循环中添加一个事件
/// @param fd 要添加事件的文件描述符
/// @param socket_event 要监听的事件类型
void CEventDispatch::AddEvent(SOCKET fd, uint8_t socket_event) {
	//创建一个 epoll_event 结构体对象 ev
    struct epoll_event ev;
	//设置ev.events的值 
	//包括 EPOLLIN可读事件、EPOLLOUT可写事件、EPOLLET边缘触发模式、EPOLLPRI高优先级事件、EPOLLERR错误事件和 EPOLLHUP挂起事件等
    ev.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLPRI | EPOLLERR | EPOLLHUP;
	//将 fd 赋值给 ev.data.fd 表示要监听的文件描述符
    ev.data.fd = fd;
	//调用 epoll_ctl() 函数将事件添加到事件循环中
    if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ev) != 0) log_error("epoll_ctl() failed, errno=%d", errno);
	//如果 epoll_ctl() 返回值不为0 表示添加事件失败打印错误信息
}


/// @brief 从事件循环中移除一个事件
/// @param fd 要移除事件的文件描述符
/// @param socket_event 要移除的事件类型
void CEventDispatch::RemoveEvent(SOCKET fd, uint8_t socket_event) {
	//调用 epoll_ctl() 函数将事件从事件循环中移除 NULL 表示不关心移除的事件信息
	int ret = epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL);
	//返回值不为 0，表示移除事件失败打印错误信息
    if (ret != 0) log_error("epoll_ctl failed, errno=%d", errno);
}

/// @brief 启动事件循环的调度过程
/// @param wait_timeout 等待事件发生时的超时时间
void CEventDispatch::StartDispatch(uint32_t wait_timeout) {
    struct epoll_event events[1024];//大小为1024的epoll_event数组 events，用于存储事件的信息
    int nfds = 0;//表示接收到的事件数量

    if (running) return;
    running = true;

	//进入事件循环的主体
    while (running) {
		// 1.使用 epoll_wait() 函数等待事件发生
        nfds = epoll_wait(m_epfd, events, 1024, wait_timeout);
		// 2.遍历接收到的事件数组，处理每个事件
        for (int i = 0; i < nfds; i++) {
			// 2-1.通过 events[i].data.fd 获取事件对应的文件描述符
            int ev_fd = events[i].data.fd;
			// 2-2.调用 FindBaseSocket 函数找到对应的 CBaseSocket 对象
            CBaseSocket* pSocket = FindBaseSocket(ev_fd);
            if (!pSocket) continue;//未找到对应的CBaseSocket对象 则继续处理下一个事件

			// 2-3.根据事件的事件类型进行 调用对应的回调函数进行处理
			#ifdef EPOLLRDHUP
			// （1）如果事件中包含 EPOLLRDHUP 标志，表示对端关闭连接，调用 OnClose() 函数处理该事件
            if (events[i].events & EPOLLRDHUP) {
				// log("On Peer Close, socket=%d, ev_fd);
				pSocket->OnClose();
			}
			#endif

			// （2）如果事件中包含 EPOLLIN 标志表示可读事件，调用 OnRead() 函数处理该事件
            if (events[i].events & EPOLLIN) {
                // log("OnRead, socket=%d\n", ev_fd);
                pSocket->OnRead();
            }

			//（3）如果事件中包含 EPOLLOUT 标志表示可写事件，调用 OnWrite() 函数处理该事件。
            if (events[i].events & EPOLLOUT) {
                // log("OnWrite, socket=%d\n", ev_fd);
                pSocket->OnWrite();
            }

			//（4）如果事件中包含 EPOLLPRI、EPOLLERR 或 EPOLLHUP 标志，表示出错或连接关闭，调用OnClose()函数处理该事件
            if (events[i].events & (EPOLLPRI | EPOLLERR | EPOLLHUP)) {
                // log("OnClose, socket=%d\n", ev_fd);
                pSocket->OnClose();
            }

			// 2-4.调用ReleaseRef()函数释放对 CBaseSocket 对象的引用
            pSocket->ReleaseRef();
        }

		// 3.调用 _CheckTimer() 函数检查定时器事件并处理
        _CheckTimer();

		// 4.调用 _CheckLoop() 函数处理循环事件
        _CheckLoop();
    }
}

//停止事件循环的调度过程
void CEventDispatch::StopDispatch() {
    running = false;
}

#endif


