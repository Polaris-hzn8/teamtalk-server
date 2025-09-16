/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: util.h
 Update Time: Mon 12 Jun 2023 17:04:05 CST
 brief: 各种工具类封装
*/

#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include "Lock.h"
#include "ostype.h"
#include "UtilPdu.h"
#include "slog_api.h"

#ifdef _WIN32
#define snprintf sprintf_s
#else
#include <time.h>
#include <stdarg.h>
#include <strings.h>
#include <pthread.h>
#include <sys/time.h>
#endif

#define _CRT_SECURE_NO_DEPRECATE // remove warning C4996,
#define NOTUSED_ARG(v) ((void)v) // remove warning C4100, unreferenced parameter

#define LOG_MODULE_IM "IM"

// 日志输出中只显示文件名
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)

#if defined(_WIN32) || defined(_WIN64)
#define log(fmt, ...) g_imlog.Info("<%s>\t<%d>\t<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define log(fmt, args...) g_imlog.Info("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#define log_debug(fmt, args...) g_imlog.Debug("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#define log_warn(fmt, args...) g_imlog.Warn("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#define log_error(fmt, args...) g_imlog.Error("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#define log_fatal(fmt, args...) g_imlog.Fatal("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
#endif

extern CSLog g_imlog;

class CRefObject
{
public:
    CRefObject();
    virtual ~CRefObject();

    void SetLock(CLock* lock) { m_lock = lock; }

    void AddRef();
    void ReleaseRef();
private:
    int     m_refCount;
    CLock*  m_lock;
};

// 字符串分割类
class CStrExplode
{
public:
    CStrExplode(char* str, char seperator);
    virtual ~CStrExplode();
    
    uint32_t GetItemCnt() { return m_item_cnt; }
    char* GetItem(uint32_t idx) { return m_item_list[idx]; }
private:
    uint32_t    m_item_cnt;     //子字符串数量
    char**      m_item_list;    //子字符串
};

std::string int2string(uint32_t user_id);
uint32_t string2int(const std::string& value);

// 获取当前系统时间
uint64_t get_tick_count();
void util_sleep(uint32_t millisecond);

//字符串操作
char* replaceStr(char* pSrc, char oldChar, char newChar);
void replace_mark(std::string& str, std::string& new_value, uint32_t& begin_pos);
void replace_mark(std::string& str, uint32_t new_value, uint32_t& begin_pos);

// server.pid
void writePid();

//将数字转换为对应十六进制字符
inline unsigned char toHex(const unsigned char& x);
//将十六进制表示的字符转换为对应的数值
inline unsigned char fromHex(const unsigned char& x);
//将字符串进行URL编码
std::string URLEncode(const std::string& sIn);
//对URL编码的字符串进行解码
std::string URLDecode(const std::string& sIn);

//获取文件的大小
int64_t get_file_size(const char* path);
//在内存中查找子字符串
const char* memfind(const char* src_str, size_t src_len, const char* sub_str, size_t sub_len, bool flag = true);

#endif
