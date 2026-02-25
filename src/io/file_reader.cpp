#include "file_reader.h"
#include <iostream>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace fakeg {
namespace io {

// FileReader类实现
FileReader::FileReader() : encoding(FileEncoding::AUTO_DETECT) {}

FileReader::FileReader(const std::string& filename, FileEncoding encoding) 
    : filename(filename), encoding(encoding) {
    open(filename, encoding);
}

FileReader::~FileReader() {
    close();
}

bool FileReader::open(const std::string& filename, FileEncoding encoding) {
    close(); // 关闭之前的文件
    
    this->filename = filename;
    this->encoding = encoding;
    
    file.open(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // 如果需要自动检测编码
    if (encoding == FileEncoding::AUTO_DETECT) {
        std::string content = readAll();
        this->encoding = detectEncoding(content);
        // 重新打开文件
        file.close();
        file.open(filename);
    }
    
    return file.is_open();
}

void FileReader::close() {
    if (file.is_open()) {
        file.close();
    }
}

bool FileReader::isOpen() const {
    return file.is_open();
}

std::ifstream& FileReader::getStream() {
    return file;
}

std::string FileReader::getFilename() const {
    return filename;
}

FileEncoding FileReader::getEncoding() const {
    return encoding;
}

size_t FileReader::getFileSize() const {
    if (!std::filesystem::exists(filename)) {
        return 0;
    }
    return std::filesystem::file_size(filename);
}

std::string FileReader::readAll() {
    if (!isOpen()) return "";
    
    std::ostringstream oss;
    oss << file.rdbuf();
    return oss.str();
}

std::vector<std::string> FileReader::readLines() {
    std::vector<std::string> lines;
    if (!isOpen()) return lines;
    
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    
    return lines;
}

FileEncoding FileReader::detectEncoding(const std::string& content) {
    // 简单的编码检测逻辑
    // 在实际项目中可能需要更复杂的检测算法
    
    // 检查UTF-8 BOM
    if (content.size() >= 3 && 
        static_cast<unsigned char>(content[0]) == 0xEF &&
        static_cast<unsigned char>(content[1]) == 0xBB &&
        static_cast<unsigned char>(content[2]) == 0xBF) {
        return FileEncoding::UTF8;
    }
    
    // 检查是否包含非ASCII字符
    bool hasNonAscii = false;
    for (char c : content) {
        if (static_cast<unsigned char>(c) > 127) {
            hasNonAscii = true;
            break;
        }
    }
    
    if (!hasNonAscii) {
        return FileEncoding::ASCII;
    }
    
    // 默认假设为UTF-8
    return FileEncoding::UTF8;
}

std::string FileReader::convertEncoding(const std::string& content, FileEncoding from, FileEncoding to) {
    // 简单的编码转换实现
    // 在实际项目中可能需要使用专门的编码转换库如iconv
    if (from == to || from == FileEncoding::ASCII || to == FileEncoding::ASCII) {
        return content;
    }
    
    // 这里只是占位符实现，实际项目中需要真正的编码转换
    return content;
}


} // namespace io
} // namespace fakeg
