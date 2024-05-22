/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: util.h
 Update Time: Mon 12 Jun 2023 17:04:05 CST
 brief: 各种工具类封装
*/

#ifndef __UTIL_H__
#define __UTIL_H__

#define _CRT_SECURE_NO_DEPRECATE // remove warning C4996,

#include "Lock.h"
#include "UtilPdu.h"
#include "ostype.h"
#include "slog/slog_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifndef _WIN32
#include <strings.h>
#endif


#include <assert.h>
#include <sys/stat.h>


#ifdef _WIN32
#define snprintf sprintf_s
#else
#include <pthread.h>
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>
#endif

/**
  * 关于 NOTUSED_ARG 宏定义的解释
  * 这是一个宏定义，用于在代码中标记未使用的参数
  * 在宏展开时会将该参数转换为(void)v，即将参数强制转换为void类型，从而在编译器中发出对该参数未使用的警告
  * 使用这个宏可以提高代码的可读性，同时也可以确保不会因为未使用的参数而导致编译警告或错误
  * 
  * 这个宏定义的目的是为了消除编译器在某些情况下可能发出的未使用参数的警告
  * 例如当某个函数的参数在某个实现中没有被使用，但仍然需要在函数签名中声明该参数，以满足接口的要求
  * 通过在实现中使用 NOTUSED_ARG 宏来标记未使用的参数，可以告诉编译器该参数是有意为之不使用的，从而避免产生警告信息
*/
#define NOTUSED_ARG(v) ((void)v) // used this to remove warning C4100, unreferenced parameter

#define LOG_MODULE_IM "IM"

extern CSLog g_imlog;

/**
 * 引用计数类 CRefObject 用于实现对象的引用计数管理
*/
class CRefObject {
public:
    CRefObject();
    virtual ~CRefObject();

    // 设置对象的锁指针，用于多线程环境下对引用计数的安全操作
    void SetLock(CLock* lock) { m_lock = lock; }
    // 增加对象的引用计数 每当有对象持有该引用计数对象时，应调用该方法增加引用计数
    void AddRef();
    // 减少对象的引用计数 当对象不再持有该引用计数对象时，应调用该方法减少引用计数。如果引用计数变为0，则可以释放该对象
    void ReleaseRef();
private:
    int m_refCount;// 引用计数，记录当前对象的引用次数
    CLock* m_lock;// 指向一个锁对象的指针，用于线程安全地管理引用计数
};


// 用于获取文件名的宏 该宏的目的是为了在日志输出中只显示文件名而不显示完整的路径
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)

// 日志输出的条件编译 根据编译平台不同设置了不同的日志输出格式
#if defined(_WIN32) || defined(_WIN64)
/* 在Windows平台下使用的日志输出格式 */
#define log(fmt, ...) g_imlog.Info("<%s>\t<%d>\t<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
/* 在非Windows平台下使用的日志输出格式 */
#define log(fmt, args...) g_imlog.Info("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#define log_debug(fmt, args...) g_imlog.Debug("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#define log_warn(fmt, args...) g_imlog.Warn("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#define log_error(fmt, args...) g_imlog.Error("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#define log_fatal(fmt, args...) g_imlog.Fatal("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#endif

//根据不同的操作系统环境获取当前的系统时间或高精度计时器值
uint64_t get_tick_count();

//用于在不同的操作系统环境下暂停/睡眠指定的时间
void util_sleep(uint32_t millisecond);

/**
 * CStrExplode 类是一个字符串分割器，用于将一个字符串按照指定的分隔符拆分成多个子字符串
 * 可以用于处理字符串分割的需求，
 * 例如将以特定分隔符分隔的文件路径、配置项等字符串拆分成单独的子字符串，便于进一步处理和使用
*/
class CStrExplode {
public:
    //待分割的字符串str，另一个是分隔符separator
    //将字符串str按照separator分隔符进行拆分，并将拆分得到的子字符串存储起来
    CStrExplode(char* str, char seperator);
    //该类的析构函数负责释放存储子字符串的内存空间
    virtual ~CStrExplode();
    
    //返回拆分后的子字符串数量，即拆分得到的子字符串的个数
    uint32_t GetItemCnt() { return m_item_cnt; }
    //返回指定索引idx处的子字符串
    char* GetItem(uint32_t idx) { return m_item_list[idx]; }
private:
    uint32_t m_item_cnt;//拆分后的子字符串的数量
    char** m_item_list;//存储拆分后的子字符串的指针列表（二维数组）
};

//工具组1
//替换字符串中的指定的字符
char* replaceStr(char* pSrc, char oldChar, char newChar);
//无符号整数转换为字符串
string int2string(uint32_t user_id);
//字符串转换为无符号整数
uint32_t string2int(const string& value);
//将字符串str中的?替换为字符串'new_value'
void replace_mark(string& str, string& new_value, uint32_t& begin_pos);
//将字符串str中的?替换为字符串'new_value'
void replace_mark(string& str, uint32_t new_value, uint32_t& begin_pos);

//工具组2
//将当前进程的PID写入到名为 server.pid 的文件中
void writePid();
//将数字转换为对应十六进制字符
inline unsigned char toHex(const unsigned char& x);
//将十六进制表示的字符转换为对应的数值
inline unsigned char fromHex(const unsigned char& x);
//将字符串进行URL编码
string URLEncode(const string& sIn);
//对URL编码的字符串进行解码
string URLDecode(const string& sIn);

//工具组3
//获取文件的大小
int64_t get_file_size(const char* path);
//在内存中查找子字符串
const char* memfind(const char* src_str, size_t src_len, const char* sub_str, size_t sub_len, bool flag = true);

#endif
