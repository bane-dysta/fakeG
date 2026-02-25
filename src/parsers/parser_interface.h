#pragma once

#include <string>
#include <memory>

#include "data/structures.h"
#include "io/file_reader.h"
#include "logger/logger.h"

namespace fakeg {
namespace parsers {

// 解析器接口基类
class ParserInterface {
protected:
    std::shared_ptr<data::ElementMap> elementMap;
    logger::Logger* logger;

public:
    ParserInterface();
    virtual ~ParserInterface() = default;

    // 设置logger
    void setLogger(logger::Logger* logger);
    
    // 核心解析方法
    virtual bool parse(io::FileReader& reader, data::ParsedData& data) = 0;
    
    // 输入验证
    virtual bool validateInput(const std::string& filename) { 
        (void)filename; // 抑制未使用参数警告
        return true; 
    }
    
    // 解析器信息
    virtual std::string getParserName() const = 0;
    virtual std::string getParserVersion() const = 0;
    virtual std::vector<std::string> getSupportedKeywords() const { return {}; }
    
protected:
    // 日志辅助方法
    void debugLog(const std::string& message) const;
    void infoLog(const std::string& message) const;
    void errorLog(const std::string& message) const;
};

} // namespace parsers
} // namespace fakeg 