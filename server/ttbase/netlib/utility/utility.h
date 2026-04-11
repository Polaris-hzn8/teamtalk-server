/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: utility.h
 Update Time: Mon 12 Jun 2023 17:04:05 CST
 brief: 各种工具类封装
*/

#ifndef __UTIL_H__
#define __UTIL_H__

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#ifdef _MSC_VER
#include <windows.h>
#else
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#endif

/// @brief 获取当前系统时间
/// @return 当前系统时间（毫秒）
uint64_t get_tick_count();

/// @brief 睡眠指定时间
/// @param millisecond 睡眠时间（毫秒）
void util_sleep(uint32_t millisecond);

// server.pid
void writePid();

#endif
