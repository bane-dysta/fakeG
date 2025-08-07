#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>

namespace fakeg {
namespace string_utils {

// 基础字符串处理函数
std::string trim(const std::string& str);
std::string ltrim(const std::string& str);
std::string rtrim(const std::string& str);

// 字符串分割函数
std::vector<std::string> split(const std::string& str, char delimiter = ' ');
std::vector<std::string> split(const std::string& str, const std::string& delimiter);

// 大小写转换
std::string toLowerCase(const std::string& str);
std::string toUpperCase(const std::string& str);

// 字符串查找和替换
bool startsWith(const std::string& str, const std::string& prefix);
bool endsWith(const std::string& str, const std::string& suffix);
bool contains(const std::string& str, const std::string& substring);
std::string replace(const std::string& str, const std::string& from, const std::string& to);
std::string replaceAll(const std::string& str, const std::string& from, const std::string& to);

// 数值转换和验证
bool isNumber(const std::string& str);
bool isInteger(const std::string& str);
double toDouble(const std::string& str, double defaultValue = 0.0);
int toInt(const std::string& str, int defaultValue = 0);
bool isValidNumber(const std::string& str);

// 引号处理
std::string removeQuotes(const std::string& str);

// 文件行处理函数
class LineProcessor {
public:
    static bool findLine(std::ifstream& file, const std::string& pattern);
    static bool findLineFromBeginning(std::ifstream& file, const std::string& pattern);
    static std::streampos getPosition(std::ifstream& file);
    static void setPosition(std::ifstream& file, std::streampos pos);
    static void resetToBeginning(std::ifstream& file);
};

// 数值解析辅助函数
template<typename T>
std::vector<T> parseValuesFromLine(const std::string& line, int maxValues = -1) {
    std::vector<T> values;
    std::istringstream iss(line);
    std::string token;
    
    while (iss >> token && (maxValues < 0 || values.size() < static_cast<size_t>(maxValues))) {
        try {
            if constexpr (std::is_same_v<T, double>) {
                values.push_back(std::stod(token));
            } else if constexpr (std::is_same_v<T, float>) {
                values.push_back(std::stof(token));
            } else if constexpr (std::is_same_v<T, int>) {
                values.push_back(std::stoi(token));
            } else if constexpr (std::is_same_v<T, long>) {
                values.push_back(std::stol(token));
            }
        } catch (...) {
            // 跳过无法转换的token
        }
    }
    
    return values;
}

} // namespace string_utils
} // namespace fakeg 