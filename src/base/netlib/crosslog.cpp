
#include "crosslog.h"

#ifdef _WIN32
    #include "yaolog.h"

    #define APP		"app"
    #define NET		"net"
    #define DEBG	"debug"
    #define ERR		"error"
    #define SOCK	"socket"

    class WinLogger : public ILogger
    {
    public:
        void levellog(LogLevel level, const char* file, int line, const char* func, const char* fmt, ...) override
        {
            char buf[2048];
            va_list args;
            va_start(args, fmt);
            vsnprintf(buf, sizeof(buf), fmt, args);
            va_end(args);

            LOG__(NET, "[%s:%d %s] %s", file, line, func, buf);
        }
    };
    // ¾²Ì¬ÊµÀý
    std::unique_ptr<ILogger> g_logger = std::make_unique<WinLogger>();
#else
    #include "slog_api.h"

    CSLog g_imlog("IM");

    class LinuxLogger : public ILogger
    {
    public:
        void levellog(LogLevel level, const char* file, int line, const char* func, const char* fmt, ...) override
        {
            char buf[2048];
            va_list args;
            va_start(args, fmt);
            vsnprintf(buf, sizeof(buf), fmt, args);
            va_end(args);

            switch (level)
            {
            case LogLevel::Debug:
                g_imlog.Debug("<%s>|<%d>|<%s>| %s", file, line, func, buf);
                break;
            case LogLevel::Info:
                g_imlog.Info("<%s>|<%d>|<%s>| %s", file, line, func, buf);
                break;
            case LogLevel::Warn:
                g_imlog.Warn("<%s>|<%d>|<%s>| %s", file, line, func, buf);
                break;
            case LogLevel::Error:
                g_imlog.Error("<%s>|<%d>|<%s>| %s", file, line, func, buf);
                break;
            case LogLevel::Fatal:
                g_imlog.Fatal("<%s>|<%d>|<%s>| %s", file, line, func, buf);
                break;
            }
        }
    };

    std::unique_ptr<ILogger> g_logger = std::make_unique<LinuxLogger>();
#endif

//#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)
//#if defined(_WIN32) || defined(_WIN64)
//#define log(fmt, ...) g_imlog.Info("<%s>\t<%d>\t<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
//#else
//#define log(fmt, args...) g_imlog.Info("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
//#define log_debug(fmt, args...) g_imlog.Debug("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
//#define log_warn(fmt, args...) g_imlog.Warn("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
//#define log_error(fmt, args...) g_imlog.Error("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
//#define log_fatal(fmt, args...) g_imlog.Fatal("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
//#endif
