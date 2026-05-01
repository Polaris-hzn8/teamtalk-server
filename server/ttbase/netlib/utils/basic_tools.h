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

/// @brief 获取单调递增的时间戳（毫秒）
/// @return 自系统启动或高精度计时器起计的毫秒数；Windows 上优先使用 QueryPerformanceCounter
uint64_t get_tick_count();

/// @brief 阻塞当前线程指定时长
/// @param millisecond 休眠毫秒数
void util_sleep(uint32_t millisecond);

/// @brief 将当前进程 ID 写入工作目录下的 server.pid 文件
/// @note 文件打开失败时触发 assert（调试构建）
void writePid();

/// @brief 查询本地文件大小
/// @param path 文件路径
/// @return 成功为字节数；stat 失败时为 (size_t)-1
size_t get_file_size(const char* path);

/// @brief 将数值 ID 编码为 URL 片段（36 进制 + 固定算法，首字符为版本前缀 '1'）
/// @param id 业务 ID
/// @return 编码后的字符串；与 urltoid 配对使用
std::string idtourl(uint32_t id);

/// @brief 将 idtourl 生成的字符串解码回 ID
/// @param url 编码串，通常首字符为 '1'
/// @return 解码后的 ID；空串或 nullptr 返回 0
uint32_t urltoid(const char* url);

/// @brief 将点分十进制 IPv4 字符串转为32 位主机序整数
/// @param ip 如 "a.b.c.d"
/// @return 网络字节序约定下的组合值（实现为 v1<<24|v2<<16|v3<<8|v4）
unsigned int ip2long(const char* ip);

/// @brief 将 32 位主机序整数转为点分十进制 IPv4 字符串
/// @param in 由 ip2long 同类布局的整型值
/// @return 指向静态缓冲区的 C字符串；非线程安全，勿长期保存指针
char* long2ip(const unsigned int in);

/// @brief 将无符号整数转为十进制字符串
/// @param user_id 用户 ID 或其它数值
/// @return 十进制文本
std::string int2string(uint32_t user_id);

/// @brief 将字符串解析为无符号整数（atoi 语义）
/// @param value 输入字符串
/// @return 转换结果，非法前缀按 atoi 规则处理
uint32_t string2int(const std::string& value);

/// @brief 将 str 中从 begin_pos 起第一个 `?` 替换为带单引号的 new_value
/// @param str 被修改的字符串
/// @param new_value 替换内容（外围会加上单引号）
/// @param begin_pos 查找起始下标，成功后会更新为替换段末尾之后的位置
void replace_mark(std::string& str, std::string& new_value, uint32_t& begin_pos);

/// @brief 将 str 中从 begin_pos 起第一个 `?` 替换为 new_value 的十进制文本
/// @param str 被修改的字符串
/// @param new_value 数值，会格式化为无引号十进制串
/// @param begin_pos 查找起始下标，成功后会更新为替换段末尾之后的位置
void replace_mark(std::string& str, uint32_t new_value, uint32_t& begin_pos);

/// @brief 将 pSrc 中所有 oldChar 替换为 newChar（原地修改）
/// @param pSrc 以空字符结尾的缓冲区，可为 nullptr
/// @param oldChar 待替换字符
/// @param newChar 新字符
/// @return pSrc；若 pSrc 为 nullptr 则返回 nullptr
char* replaceStr(char* pSrc, char oldChar, char newChar);

#endif
