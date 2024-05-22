/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: slog_api.cpp
 Update Time: Sun 11 Jun 2023 11:21:34 CST
 brief: 
    SLog模块依赖于log4cxx，实际是对log4cxx的进一步封装 从而实现一个简单的日志系统
    CSLogObject 定义了一组虚函数作为日志操作的接口
    CLog4CXX 是继承自 CSLogObject 的子类，使用 log4cxx 库来实现具体的日志记录功能
*/

#include "slog_api.h"
#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/helpers/exception.h"
#include <stdarg.h>
using namespace log4cxx;

#define MAX_LOG_LENGTH   1024 * 10

class CSLogObject {
public:
    CSLogObject(const char* module_name, int delay = WATCH_DELAY_TIME) {}
    virtual ~CSLogObject() {}
    
    virtual void Trace(const char* loginfo) {}
    virtual void Debug(const char* loginfo) {}
    virtual void Info(const char* loginfo) {}
    virtual void Warn(const char* loginfo) {}
    virtual void Error(const char* loginfo) {}
    virtual void Fatal(const char* loginfo) {}
};

class CLog4CXX : public CSLogObject {
public:
    CLog4CXX(const char* module_name, int delay = WATCH_DELAY_TIME);
    virtual ~CLog4CXX();
    
    void Trace(const char* loginfo);
    void Debug(const char* loginfo);
    void Info(const char* loginfo);
    void Warn(const char* loginfo);
    void Error(const char* loginfo);
    void Fatal(const char* loginfo);
private:
    LoggerPtr m_logger;
};

// 通过 CLog4CXX 类的构造函数初始化日志记录器对象 m_logger
// 1.使用了成员初始化列表来调用基类 CSLogObject 的构造函数，并将 module_name 和 delay 作为参数传递给基类的构造函数
CLog4CXX::CLog4CXX(const char* module_name, int delay) : CSLogObject(module_name, delay) {
    // 2.使用log4cxx提供的configureAndWatch方法 从log4cxx.properties 文件中读取配置信息，并指定了刷新配置的时间间隔delay
    PropertyConfigurator::configureAndWatch("log4cxx.properties", delay);
    // 3.使用log4cxx的Logger::getLogger方法，根据module_name获取与之对应的日志记录器对象，并将其赋值给成员变量m_logger
    m_logger = Logger::getLogger(module_name);
}

CLog4CXX::~CLog4CXX() {}

void CLog4CXX::Trace(const char *loginfo) {
    m_logger->trace(loginfo);
    //LOG4CXX_TRACE(m_logger, loginfo);
}

void CLog4CXX::Debug(const char *loginfo) {
    m_logger->debug(loginfo);
}

void CLog4CXX::Info(const char *loginfo) {
    m_logger->info(loginfo);
}

void CLog4CXX::Warn(const char *loginfo) {
    m_logger->warn(loginfo);
}

void CLog4CXX::Error(const char *loginfo) {
    m_logger->error(loginfo);
}

void CLog4CXX::Fatal(const char *loginfo) {
    m_logger->fatal(loginfo);
}


// 在CSLog类的构造函数中创建一个CLog4CXX 对象，并将其作为日志记录器对象赋值给成员变量m_log，以便在后续的日志输出操作中使用
// 通过使用基类CSLogObject的指针类型作为成员变量 m_log 的类型
// 可以实现多态性，即通过基类指针调用派生类的成员函数，这样做可以提高代码的灵活性和可扩展性
CSLog::CSLog(const char* module_name, int delay) {
    m_log = new CLog4CXX(module_name, delay);
}

CSLog::~CSLog() {
    delete m_log;
}

// 接收格式化字符串和变长参数，将其格式化为完整的日志消息，并调用相应的日志记录器对象的 Trace 函数进行日志输出
void CSLog::Trace(const char *format, ...) {
    // 声明一个变量args用于存储可变参数列表的信息
    va_list args;
    // 将args初始化为传递给函数的可变参数列表的起始位置，format是可变参数列表中的第一个参数
    va_start(args, format);
    // 声明一个字符数组szBuffer，用于存储格式化后的日志消息
    char szBuffer[MAX_LOG_LENGTH];
    // 使用 vsnprintf 函数将可变参数列表中的参数按照指定的格式format格式化为字符串，并将结果存储在szBuffer中
    vsnprintf(szBuffer, sizeof(szBuffer), format, args);
    // 可变参数获取结束 清理args
    va_end(args);
    // 调用m_log指向的对象（在此情况下是 CLog4CXX 对象）的Trace函数，将格式化后的日志消息作为参数传递给它，实现日志输出
    m_log->Trace(szBuffer);
}

void CSLog::Debug(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char szBuffer[MAX_LOG_LENGTH];
    vsnprintf(szBuffer, sizeof(szBuffer) , format, args);
    va_end(args);
    m_log->Debug(szBuffer);
}

void CSLog::Info(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char szBuffer[MAX_LOG_LENGTH];
    vsnprintf(szBuffer, sizeof(szBuffer), format, args);
    va_end(args);
    m_log->Info(szBuffer);
}

void CSLog::Warn(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char szBuffer[MAX_LOG_LENGTH];
    vsnprintf(szBuffer, sizeof(szBuffer), format, args);
    va_end(args);
    m_log->Warn(szBuffer);
}

void CSLog::Error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char szBuffer[MAX_LOG_LENGTH];
    vsnprintf(szBuffer, sizeof(szBuffer), format, args);
    va_end(args);
    m_log->Error(szBuffer);
}

void CSLog::Fatal(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char szBuffer[MAX_LOG_LENGTH];
    vsnprintf(szBuffer, sizeof(szBuffer), format, args);
    va_end(args);
    m_log->Fatal(szBuffer);
}

/*
int main(int argc, char* argv[])
{
    CLog a("test2");

    for (int i = 0; i < 100000; i++) {
        a.Warn("aaa,%s", "bbb");
    }
    //CLog4CXX b("test2");
    //b.DEBUG("bbbbbb");
    return 0;
}
*/

