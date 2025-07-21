/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: Lock.h
 Update Time: Mon 12 Jun 2023 17:43:23 CST
 brief: 锁机制的封装
*/

#ifndef __LOCK_H__
#define __LOCK_H__

#include "ostype.h"

////////////////////////CLock/////////////////////////////////////
/**
 * CLock 类是一个互斥锁的封装类
 * 
 * CLock：用于获取互斥锁。如果互斥锁当前不可用，调用线程将被阻塞，直到互斥锁可用为止
 * ~CLock：用于释放互斥锁，使其可被其他线程获取
 * getMutex：返回互斥锁对象，以便对其进行直接操作（例如在条件变量中使用）
 * try_lock（非Windows系统）：尝试获取互斥锁，如果互斥锁当前不可用，则立即返回失败
 * 
 * 在Windows系统下（_WIN32宏定义为真），使用临界区（CRITICAL_SECTION）来实现互斥锁
 * 而在非Windows系统下，则使用 pthread 库提供的互斥锁（pthread_mutex_t）
 * 
 * 这样的封装使得代码在不同操作系统上具有可移植性，因为在不同系统上使用不同的互斥锁实现方式
*/
class CLock
{
public:
    CLock();
    virtual ~CLock();

    void lock();
    void unlock();
    pthread_mutex_t& getMutex() { return m_lock; }
#ifndef _WIN32
    virtual bool try_lock();
#endif
private:
#ifdef _WIN32
	CRITICAL_SECTION m_critical_section;
#else
    pthread_mutex_t m_lock;
#endif
};

///////////////////// CAutoLock//////////////////////////////////////
/**
 * CAutoLock 类是一个自动锁的封装类，用于简化对互斥锁的加锁和解锁操作
 * 封装了普通局部锁 利用c++语法局部变量离开作用域自动析构原理，实现自动解锁（牺牲了一定效率）
 * 
 * 通过构造函数和析构函数的调用，实现了自动获取锁和自动释放锁的功能，避免了手动管理锁的操作
 * CAutoLock(CLock* pLock)：接受一个CLock对象指针作为参数，用于初始化CAutoLock对象
 * 在构造函数中会调用pLock对象的lock()方法，自动获取锁
 * ~CAutoLock()：在对象销毁时自动调用
 * 在析构函数中会调用m_pLock对象的unlock()方法自动释放锁
*/
class CAutoLock {
public:
    CAutoLock(CLock* pLock);
    virtual ~CAutoLock();
private:
    CLock* m_pLock;
};

//////////////////////WIN32//////////////////////////////////////////
#ifndef _WIN32
///////////////////// CRWLock//////////////////////////////////////
/**
 * CRWLock 类是一个读写锁的封装类，用于实现多读单写的并发控制
 * 
 * CRWLock(); 构造函数用于初始化读写锁对象。
 * virtual ~CRWLock(); 析构函数用于销毁读写锁对象。
 * 
 * void rlock(); 获取读锁，允许多个线程同时获取读锁，当有线程持有写锁时，其他线程无法获取写锁。
 * void wlock(); 获取写锁，只允许一个线程获取写锁，当有线程持有读锁或写锁时，其他线程无法获取读锁或写锁。
 * void unlock(); 释放读锁或写锁
 * bool try_rlock(); 尝试获取读锁，如果能成功获取则返回 true，否则返回 false
 * bool try_wlock(); 尝试获取写锁，如果能成功获取则返回 true，否则返回 false
 * 
 * 使用读写锁可以实现多线程读取共享数据，而在写操作时需要独占访问共享数据的情况下，保证数据的一致性和并发安全性
*/
class CRWLock {
public:
    CRWLock();
    virtual ~CRWLock();

    void rlock();
    void wlock();
    void unlock();
    bool try_rlock();
    bool try_wlock();
private:
    pthread_rwlock_t m_lock;
};

///////////////////// CAutoRWLock//////////////////////////////////////
/**
 * CAutoRWLock 类是一个自动读写锁的封装类，用于简化对读写锁的加锁和解锁操作
 * 封装了读写局部锁，利用c++语法局部变量离开作用域自动析构原理，实现自动解锁（牺牲了一定效率）
 * 
 * 构造函数，接收一个CRWLock对象指针和一个布尔值参数 当布尔值参数为true时默认获取读锁；当布尔值参数为false时获取写锁。
 * 析构函数，用于释放自动锁对象，自动调用解锁操作
 * 
 * 使用 CAutoRWLock 类可以在进入作用域时自动获取读锁或写锁
 * 并在离开作用域时自动释放锁，避免手动管理锁的加锁和解锁操作，从而简化代码并确保正确的加锁和解锁顺序
*/
class CAutoRWLock {
public:
    CAutoRWLock(CRWLock* pLock, bool bRLock = true);
    virtual ~CAutoRWLock();
private:
    CRWLock* m_pLock;
};

#endif  

#endif
