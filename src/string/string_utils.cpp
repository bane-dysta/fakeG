#include "string_utils.h"
#include <cctype>

namespace fakeg {
namespace string_utils {

// 基础字符串处理函数
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

std::string ltrim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    return str.substr(first);
}

std::string rtrim(const std::string& str) {
    size_t last = str.find_last_not_of(" \t\r\n");
    if (last == std::string::npos) return "";
    return str.substr(0, last + 1);
}

// 字符串分割函数
std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        std::string trimmed = trim(token);
        if (!trimmed.empty()) {
            tokens.push_back(trimmed);
        }
    }
    
    return tokens;
}

std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);
    
    while (end != std::string::npos) {
        std::string token = trim(str.substr(start, end - start));
        if (!token.empty()) {
            tokens.push_back(token);
        }
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    
    std::string lastToken = trim(str.substr(start));
    if (!lastToken.empty()) {
        tokens.push_back(lastToken);
    }
    
    return tokens;
}

// 大小写转换
std::string toLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string toUpperCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

// 字符串查找和替换
bool startsWith(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && 
           str.substr(0, prefix.size()) == prefix;
}

bool endsWith(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && 
           str.substr(str.size() - suffix.size()) == suffix;
}

bool contains(const std::string& str, const std::string& substring) {
    return str.find(substring) != std::string::npos;
}

std::string replace(const std::string& str, const std::string& from, const std::string& to) {
    std::string result = str;
    size_t pos = result.find(from);
    if (pos != std::string::npos) {
        result.replace(pos, from.length(), to);
    }
    return result;
}

std::string replaceAll(const std::string& str, const std::string& from, const std::string& to) {
    std::string result = str;
    size_t pos = 0;
    while ((pos = result.find(from, pos)) != std::string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    return result;
}

// 数值转换和验证
bool isNumber(const std::string& str) {
    if (str.empty()) return false;
    
    size_t start = 0;
    if (str[0] == '+' || str[0] == '-') start = 1;
    
    bool hasDecimal = false;
    bool hasDigit = false;
    
    for (size_t i = start; i < str.size(); i++) {
        if (std::isdigit(str[i])) {
            hasDigit = true;
        } else if (str[i] == '.' && !hasDecimal) {
            hasDecimal = true;
        } else if ((str[i] == 'e' || str[i] == 'E') && i > start && hasDigit) {
            // 科学记数法
            if (i + 1 < str.size() && (str[i + 1] == '+' || str[i + 1] == '-')) i++;
        } else if (str[i] == 'D' && i > start && hasDigit) {
            // BDF的D记数法
            if (i + 1 < str.size() && (str[i + 1] == '+' || str[i + 1] == '-')) i++;
        } else {
            return false;
        }
    }
    
    return hasDigit;
}

bool isInteger(const std::string& str) {
    if (str.empty()) return false;
    
    size_t start = 0;
    if (str[0] == '+' || str[0] == '-') start = 1;
    
    for (size_t i = start; i < str.size(); i++) {
        if (!std::isdigit(str[i])) {
            return false;
        }
    }
    
    return start < str.size();
}

double toDouble(const std::string& str, double defaultValue) {
    try {
        std::string processedStr = replaceAll(str, "D", "E"); // 处理BDF的D记数法
        return std::stod(processedStr);
    } catch (...) {
        return defaultValue;
    }
}

int toInt(const std::string& str, int defaultValue) {
    try {
        return std::stoi(str);
    } catch (...) {
        return defaultValue;
    }
}

bool isValidNumber(const std::string& str) {
    if (str.empty()) return false;
    
    std::string cleanStr = str;
    // 处理科学记数法中的D
    std::replace(cleanStr.begin(), cleanStr.end(), 'D', 'E');
    std::replace(cleanStr.begin(), cleanStr.end(), 'd', 'e');
    
    try {
        std::stod(cleanStr);
        return true;
    } catch (...) {
        return false;
    }
}

std::string removeQuotes(const std::string& str) {
    std::string result = str;
    
    // 去除前导和尾随空白
    size_t start = result.find_first_not_of(" \t");
    if (start != std::string::npos) {
        result = result.substr(start);
    }
    
    size_t end = result.find_last_not_of(" \t");
    if (end != std::string::npos) {
        result = result.substr(0, end + 1);
    }
    
    // 去除引号
    if (result.length() >= 2) {
        if ((result.front() == '\'' && result.back() == '\'') ||
            (result.front() == '"' && result.back() == '"')) {
            result = result.substr(1, result.length() - 2);
        }
    }
    
    return result;
}

// LineProcessor类实现
bool LineProcessor::findLine(std::ifstream& file, const std::string& pattern) {
    std::string line;
    while (std::getline(file, line)) {
        if (line.find(pattern) != std::string::npos) {
            return true;
        }
    }
    return false;
}

bool LineProcessor::findLineFromBeginning(std::ifstream& file, const std::string& pattern) {
    resetToBeginning(file);
    return findLine(file, pattern);
}

std::streampos LineProcessor::getPosition(std::ifstream& file) {
    return file.tellg();
}

void LineProcessor::setPosition(std::ifstream& file, std::streampos pos) {
    file.clear();
    file.seekg(pos);
}

void LineProcessor::resetToBeginning(std::ifstream& file) {
    file.clear();
    file.seekg(0, std::ios::beg);
}

} // namespace string_utils
} // namespace fakeg 