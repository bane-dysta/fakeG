#include "logger.h"
#include <iomanip>

namespace fakeg {
namespace logger {

// 全局logger实例定义
Logger globalLogger;

Logger::Logger(bool debug, LogLevel level) 
    : debugMode(debug), minLevel(level), prefix("") {}

void Logger::setDebugMode(bool enable) {
    debugMode = enable;
    if (enable) {
        minLevel = LogLevel::DEBUG;
    }
}

bool Logger::isDebugMode() const {
    return debugMode;
}

void Logger::setMinLevel(LogLevel level) {
    minLevel = level;
}

void Logger::setPrefix(const std::string& prefix) {
    this->prefix = prefix;
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < minLevel) return;
    
    std::string levelStr;
    switch (level) {
        case LogLevel::DEBUG:   levelStr = "[DEBUG]"; break;
        case LogLevel::INFO:    levelStr = "[INFO]"; break;
        case LogLevel::WARNING: levelStr = "[WARNING]"; break;
        case LogLevel::ERROR:   levelStr = "[ERROR]"; break;
    }
    
    if (!prefix.empty()) {
        std::cout << prefix << " ";
    }
    
    if (level != LogLevel::INFO) {
        std::cout << levelStr << " ";
    }
    
    std::cout << message << std::endl;
}

void Logger::debug(const std::string& message) {
    if (debugMode) {
        log(LogLevel::DEBUG, message);
    }
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

} // namespace logger
} // namespace fakeg 