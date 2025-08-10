/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: slog_api.h
 Update Time: Sun 11 Jun 2023 11:22:26 CST
 brief: 封装log4cxx为SLog
*/

#ifndef __slog__slog_api__
#define __slog__slog_api__

#include <stdio.h>

#define WATCH_DELAY_TIME     10 * 1000

class CSLogObject;
class CSLog
{
public:
    CSLog(const char* module_name, int delay = WATCH_DELAY_TIME);
    virtual ~CSLog();
    
    void Trace(const char* format, ...);
    void Debug(const char* format, ...);
    void Info(const char* format, ...);
    void Warn(const char* format, ...);
    void Error(const char* format, ...);
    void Fatal(const char* format, ...);
private:
    CSLogObject* m_log;
};

#endif /* defined(__slog__slog_api__) */
