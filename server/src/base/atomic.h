/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: atomic.h
 Update Time: Sun 11 Jun 2023 14:45:18 CST
 brief:
    原子操作的简单实现 用于提供在多线程环境下的原子操作功能
    主要定义了一些宏和类型来实现原子操作
*/

#ifndef _ATOMIC_H_
#define _ATOMIC_H_

//宏用于将val值加到给定指针所指向的内存位置（ATOMIC_ADD）
#define ATOMIC_ADD(src_ptr, v) (void)__sync_add_and_fetch(src_ptr, v)

//宏用于val值加到给定指针所指向的内存位置，并返回相加后的结果（ATOMIC_ADD_AND_FETCH）
#define ATOMIC_ADD_AND_FETCH(src_ptr, v) __sync_add_and_fetch(src_ptr, v)

//宏用于将val值从给定指针所指向的内存位置减去，并返回相减后的结果（ATOMIC_SUB_AND_FETCH）
#define ATOMIC_SUB_AND_FETCH(src_ptr, v) __sync_sub_and_fetch(src_ptr, v)

//宏用于获取给定指针所指向的内存位置的值，并返回该值（ATOMIC_FETCH）
#define ATOMIC_FETCH(src_ptr) __sync_add_and_fetch(src_ptr, 0)

//宏用于将给定值设置到给定指针所指向的内存位置（ATOMIC_SET）
#define ATOMIC_SET(src_ptr, v) (void)__sync_bool_compare_and_swap(src_ptr, *(src_ptr), v)


// 定义了atomic_t类型 是一个long类型的变量
// 并使用volatile关键字修饰 以确保在多线程环境中对该变量的访问是原子的
typedef volatile long atomic_t;

#endif
