/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: atomic.h
 Update Time: Sun 11 Jun 2023 14:45:18 CST
 brief: 原子操作
    1.多线程环境下的原子操作
    2.访问共享变量时保证读写的原子性与线程安全性
*/

#ifndef _ATOMIC_H_
#define _ATOMIC_H_

// src_ptr + v
#define ATOMIC_ADD(src_ptr, v) (void)__sync_add_and_fetch(src_ptr, v)

// src_ptr + v
#define ATOMIC_ADD_AND_FETCH(src_ptr, v) __sync_add_and_fetch(src_ptr, v)

// src_ptr - v
#define ATOMIC_SUB_AND_FETCH(src_ptr, v) __sync_sub_and_fetch(src_ptr, v)

// src_ptr get
#define ATOMIC_FETCH(src_ptr) __sync_add_and_fetch(src_ptr, 0)

// src_ptr set
#define ATOMIC_SET(src_ptr, v) (void)__sync_bool_compare_and_swap(src_ptr, *(src_ptr), v)

typedef volatile long atomic_t;// 原子访问

#endif
