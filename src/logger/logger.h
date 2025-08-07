#pragma once

#include <iostream>
#include <string>
#include <sstream>

namespace fakeg {
namespace logger {

// 日志级别枚举
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3
};

// Logger类
class Logger {
private:
    bool debugMode;
    LogLevel minLevel;
    std::string prefix;
    
public:
    Logger(bool debug = false, LogLevel level = LogLevel::INFO);
    
    // 设置调试模式
    void setDebugMode(bool enable);
    bool isDebugMode() const;
    
    // 设置最小日志级别
    void setMinLevel(LogLevel level);
    
    // 设置前缀
    void setPrefix(const std::string& prefix);
    
    // 基础输出方法
    void log(LogLevel level, const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    
    // 便捷的流式输出接口
    template<typename T>
    void debug(const T& message) {
        if (debugMode && minLevel <= LogLevel::DEBUG) {
            std::ostringstream oss;
            oss << message;
            debug(oss.str());
        }
    }
    
    template<typename T>
    void info(const T& message) {
        if (minLevel <= LogLevel::INFO) {
            std::ostringstream oss;
            oss << message;
            info(oss.str());
        }
    }
};

// 全局logger实例
extern Logger globalLogger;

// 便捷的宏定义
#define LOG_DEBUG(msg) fakeg::logger::globalLogger.debug(msg)
#define LOG_INFO(msg) fakeg::logger::globalLogger.info(msg)
#define LOG_WARNING(msg) fakeg::logger::globalLogger.warning(msg)
#define LOG_ERROR(msg) fakeg::logger::globalLogger.error(msg)

} // namespace logger
} // namespace fakeg 