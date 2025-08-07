#pragma once

#include <string>
#include <memory>
#include "../data/structures.h"
#include "../io/file_reader.h"
#include "../io/gaussian_writer.h"
#include "../parsers/parser_interface.h"
#include "../logger/logger.h"

namespace fakeg {
namespace app {

// 主应用程序类
class FakeGApp {
private:
    // 应用程序状态
    std::string inputFilename;
    std::string outputFilename;
    bool debugMode;
    
    // 程序信息
    std::string programName;
    std::string programVersion;
    std::string authorInfo;
    
    // 核心组件
    std::unique_ptr<parsers::ParserInterface> parser;
    mutable logger::Logger appLogger;  // 声明为 mutable
    io::GaussianWriter writer;
    
public:
    FakeGApp();
    explicit FakeGApp(std::unique_ptr<parsers::ParserInterface> parser);
    
    // 设置解析器
    void setParser(std::unique_ptr<parsers::ParserInterface> parser);
    
    // 配置方法
    void setProgramInfo(const std::string& name, const std::string& version, const std::string& author);
    void setDebugMode(bool enable);
    void setInputFile(const std::string& filename);
    void setOutputFile(const std::string& filename);
    
    // 核心功能
    bool initialize();
    bool processFile();
    bool validateOutput();
    
    // 主运行方法
    bool run(int argc, char* argv[]);
    bool run(const std::string& inputFilename, bool debugMode = false);
    
    // 状态查询
    std::string getInputFile() const;
    std::string getOutputFile() const;
    bool isDebugMode() const;
    
    // 帮助和版本信息
    void printHelp() const;
    void printVersion() const;
    
private:
    // 内部方法
    bool parseCommandLineArgs(int argc, char* argv[]);
    bool setupOutput();
    void showProgressInfo(const data::ParsedData& data);  // 去掉 const
    void showErrorInfo(const std::string& error);         // 去掉 const
};

} // namespace app
} // namespace fakeg 