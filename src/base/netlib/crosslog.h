
#ifndef _CROSSS_LOG_H_
#define _CROSSS_LOG_H_

/**
 * @file crosslog.h
 * @author polaris_hzn8
 * @brief 跨平台日志库
 * @version 0.1
 * @date 2025-11-05
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <string>
#include <memory>
#include <cstdarg>

enum class LogLevel
{
    Debug = 0,
    Info,
    Warn,
    Error,
    Fatal
};

// 抽象接口
class ILogger
{
public:
    virtual ~ILogger() = default;
    virtual void levellog(LogLevel level, const char* file, int line, const char* func, const char* fmt, ...) = 0;
};
// 全局日志指针
extern std::unique_ptr<ILogger> g_logger;

// 辅助宏
#define log_info(fmt, ...)  g_logger->levellog(LogLevel::Info,  __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define log_debug(fmt, ...) g_logger->levellog(LogLevel::Debug, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...)  g_logger->levellog(LogLevel::Warn,  __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define log_error(fmt, ...) g_logger->levellog(LogLevel::Error, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#define log_fatal(fmt, ...) g_logger->levellog(LogLevel::Fatal, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)

#endif // _CROSSS_LOG_H_
