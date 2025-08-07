#pragma once

#include <string>
#include <fstream>
#include <vector>

namespace fakeg {
namespace io {

// 文件编码类型
enum class FileEncoding {
    UTF8,
    GBK,
    ASCII,
    AUTO_DETECT
};

// 文件读取器类
class FileReader {
private:
    std::string filename;
    FileEncoding encoding;
    std::ifstream file;
    
    // 编码检测和转换
    FileEncoding detectEncoding(const std::string& content);
    std::string convertEncoding(const std::string& content, FileEncoding from, FileEncoding to);
    
public:
    FileReader();
    FileReader(const std::string& filename, FileEncoding encoding = FileEncoding::AUTO_DETECT);
    ~FileReader();
    
    // 文件操作
    bool open(const std::string& filename, FileEncoding encoding = FileEncoding::AUTO_DETECT);
    void close();
    bool isOpen() const;
    
    // 获取文件流的引用（用于解析器）
    std::ifstream& getStream();
    
    // 文件信息
    std::string getFilename() const;
    FileEncoding getEncoding() const;
    size_t getFileSize() const;
    
    // 读取整个文件内容
    std::string readAll();
    
    // 按行读取
    std::vector<std::string> readLines();
};

// 命令行参数处理类
class ArgumentParser {
private:
    std::vector<std::string> args;
    std::string programName;
    
public:
    ArgumentParser(int argc, char* argv[]);
    
    // 获取参数
    bool hasFlag(const std::string& flag) const;
    std::string getValue(const std::string& option, const std::string& defaultValue = "") const;
    std::string getPositionalArg(size_t index, const std::string& defaultValue = "") const;
    size_t getPositionalArgCount() const;
    
    // 程序信息
    std::string getProgramName() const;
    
    // 帮助信息
    void printUsage(const std::string& description, 
                   const std::vector<std::string>& options = {},
                   const std::vector<std::string>& examples = {}) const;
};

} // namespace io
} // namespace fakeg 