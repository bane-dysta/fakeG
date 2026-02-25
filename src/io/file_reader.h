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

} // namespace io
} // namespace fakeg
