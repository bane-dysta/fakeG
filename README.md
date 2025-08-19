# FakeG - 模块化量子化学输出转换工具

> 内容大多由Claude生成，彩红屁勿信，技术方面相对可信。

## 项目概述

FakeG是一个模块化的C++项目，用于将各种量子化学软件的结构优化输出文件转换为伪Gaussian格式文件，从而可以使用gview可视化结构优化过程。

**当前支持的格式：**
- **AMESP** → AfakeG可执行文件
- **BDF** → BfakeG可执行文件
- **XYZ/TRJ轨迹** → XfakeG可执行文件
- **XTB Gaussian格式** → XtbfakeG可执行文件

> 关于orca，可移步sobereva老师开发的[OfakeG](http://sobereva.com/498)。

## 主要特点

- **模块化设计**：代码组织清晰，各模块职责明确
- **可扩展架构**：通过接口设计轻松添加新解析器
- **跨平台支持**：支持Linux和Windows（通过交叉编译）
- **静态链接**：多种链接选项，实现最大兼容性
- **完整日志系统**：多级日志系统，支持调试
- **虚原子支持**：将未知元素处理为Bq（虚原子）

## 项目结构

```
FakeG/
├── src/                    # 源代码目录
│   ├── data/              # 数据结构模块
│   │   ├── structures.h   # 数据结构定义
│   │   └── structures.cpp # 元素映射和数据结构
│   ├── io/                # IO模块
│   │   ├── file_reader.h/cpp    # 文件读取，支持编码检测
│   │   └── gaussian_writer.h/cpp # Gaussian格式输出
│   ├── logger/            # 日志模块
│   │   ├── logger.h       # 多级日志系统
│   │   └── logger.cpp
│   ├── string/            # 字符串工具模块
│   │   ├── string_utils.h # 字符串处理工具
│   │   └── string_utils.cpp
│   ├── parsers/           # 解析器模块
│   │   ├── parser_interface.h/cpp  # 解析器基础接口
│   │   ├── amesp_parser.h/cpp      # AMESP格式解析器
│   │   ├── bdf_parser.h/cpp        # BDF格式解析器
│   │   ├── xyz_parser.h/cpp        # XYZ/TRJ轨迹解析器
│   │   └── xtb_parser.h/cpp        # XTB Gaussian格式解析器
│   └── main/              # 主程序模块
│       ├── fake_g_app.h/cpp        # 应用程序框架
│       ├── afake_g.cpp             # AfakeG主程序
│       ├── bfake_g.cpp             # BfakeG主程序
│       ├── xfake_g.cpp             # XfakeG主程序
│       └── xtbfake_g.cpp           # XtbfakeG主程序
├── config/                # 配置文件
├── build.sh              # 通用构建脚本
├── build_windows.sh      # Windows交叉编译脚本
├── CMakeLists.txt        # CMake构建配置
└── README.md             # 项目文档
```

## 构建项目

### 系统要求

- CMake 3.16+
- C++20兼容编译器（GCC 10+, Clang 12+, MSVC 2019+）
- Windows交叉编译需要：mingw-w64

### 快速构建

```bash
# 默认Linux构建（动态链接）
./build.sh

# 静态链接，提高可移植性
./build.sh linux-static

# 完全静态链接（包括glibc）
./build.sh linux-full

# Windows交叉编译
./build.sh windows

# 清理构建文件
./build.sh clean

# 显示帮助
./build.sh help
```

### 手动构建

```bash
# 创建构建目录
mkdir build && cd build

# 配置（多种选项可用）
cmake ..                          # 默认动态链接
cmake -DSTATIC_LINKING=ON ..      # 部分静态链接
cmake -DFULL_STATIC=ON ..         # 完全静态链接
cmake -DWINDOWS_BUILD=ON ..       # Windows交叉编译

# 构建
make -j$(nproc)
```

## 使用方法

### AfakeG (AMESP格式转换)

```bash
# 交互模式（无参数）
./afakeg

# 命令行模式
./afakeg input.aop
./afakeg input.aop --debug
./afakeg input.aop -o output.log
./afakeg --help
```

### BfakeG (BDF格式转换)

```bash
# 交互模式
./bfakeg

# 命令行模式
./bfakeg input.out
./bfakeg input.out --debug
```

### XfakeG (XYZ/TRJ轨迹转换)

```bash
# 交互模式
./xfakeg

# 命令行模式
./xfakeg trajectory.xyz
./xfakeg trajectory.trj --debug

# 支持能量提取的格式：
# molclus: Energy =   -147.48410656 a.u.  #Cluster:    1
# xtb:     energy: -149.706157544781 gnorm: 0.499531841458 xtb: 6.7.0 (75e6a61)
```

### XtbfakeG (XTB Gaussian格式转换)

```bash
# 交互模式
./xtbfakeg

# 命令行模式
./xtbfakeg g98.out
./xtbfakeg g98.out --debug

# 支持的输入格式：
# - XTB生成的Gaussian 98格式频率输出
# - 包含标准定向坐标、频率信息和正则坐标
# - 自动设置标准温度(298.15K)和压力(1.0atm)以确保gview兼容性
```

## 编写新解析器

### 架构概述

FakeG架构使用**直接解析器注入**而非工厂模式。每个可执行文件都链接到特定的解析器，使设计简单明确。

### 分步指南

#### 1. 创建解析器头文件

创建 `src/parsers/your_parser.h`：

```cpp
#pragma once

#include "parser_interface.h"
#include "../string/string_utils.h"

namespace fakeg {
namespace parsers {

class YourParser : public ParserInterface {
public:
    YourParser();
    
    // 必需的接口方法
    bool parse(io::FileReader& reader, data::ParsedData& data) override;
    bool validateInput(const std::string& filename) override;
    
    std::string getParserName() const override;
    std::string getParserVersion() const override;
    std::vector<std::string> getSupportedKeywords() const override;

private:
    // 你的解析方法
    bool parseOptimizationSteps(std::ifstream& file, data::ParsedData& data);
    bool parseFrequencies(std::ifstream& file, data::ParsedData& data);
    bool parseThermoData(std::ifstream& file, data::ParsedData& data);
    
    // 辅助方法
    void parseGeometry(std::ifstream& file, std::vector<data::Atom>& atoms);
    double parseEnergy(const std::string& line);
};

} // namespace parsers
} // namespace fakeg
```

#### 2. 实现解析器逻辑

创建 `src/parsers/your_parser.cpp`：

```cpp
#include "your_parser.h"
#include "../string/string_utils.h"
#include <sstream>

namespace fakeg {
namespace parsers {

YourParser::YourParser() = default;

bool YourParser::parse(io::FileReader& reader, data::ParsedData& data) {
    std::ifstream& file = reader.getStream();
    
    infoLog("开始解析YourFormat文件");
    
    // 重置文件位置
    string_utils::LineProcessor::resetToBeginning(file);
    
    // 检查优化
    if (string_utils::LineProcessor::findLine(file, "YOUR_OPT_KEYWORD")) {
        data.hasOpt = true;
        infoLog("发现几何优化");
        if (!parseOptimizationSteps(file, data)) {
            errorLog("优化解析失败");
            return false;
        }
    }
    
    // 解析频率
    if (parseFrequencies(file, data)) {
        data.hasFreq = true;
        infoLog("频率解析完成");
    }
    
    // 解析热力学数据
    if (parseThermoData(file, data)) {
        infoLog("热力学数据解析完成");
    }
    
    return !data.optSteps.empty();
}

bool YourParser::validateInput(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        errorLog("无法打开文件: " + filename);
        return false;
    }
    // 用户指定的文件总是用于转换
    return true;
}

std::string YourParser::getParserName() const {
    return "YourParser";
}

std::string YourParser::getParserVersion() const {
    return "1.0.0";
}

std::vector<std::string> YourParser::getSupportedKeywords() const {
    return {"YOUR_OPT_KEYWORD", "YOUR_FREQ_KEYWORD", "YOUR_THERMO_KEYWORD"};
}

bool YourParser::parseOptimizationSteps(std::ifstream& file, data::ParsedData& data) {
    string_utils::LineProcessor::resetToBeginning(file);
    
    std::string line;
    while (std::getline(file, line)) {
        if (string_utils::contains(line, "YOUR_STEP_MARKER")) {
            data::OptStep step;
            step.stepNumber = extractStepNumber(line);
            step.energy = 0.0;
            
            // 解析此步骤的几何结构
            parseGeometry(file, step.atoms);
            
            // 解析能量
            step.energy = parseEnergy(line);
            
            // 解析收敛数据
            // ... 你的收敛解析逻辑
            
            if (!step.atoms.empty()) {
                data.optSteps.push_back(step);
                debugLog("添加步骤 " + std::to_string(step.stepNumber) + 
                        "，包含 " + std::to_string(step.atoms.size()) + " 个原子");
            }
        }
    }
    
    return !data.optSteps.empty();
}

// 实现其他解析方法...

} // namespace parsers
} // namespace fakeg
```

#### 3. 创建主可执行文件

创建 `src/main/yourfake_g.cpp`：

```cpp
#include <iostream>
#include <string>
#include "fake_g_app.h"
#include "../parsers/your_parser.h"
#include "../string/string_utils.h"

using namespace fakeg;

int main(int argc, char* argv[]) {
    // 创建 YOUR 解析器
    auto parser = std::make_unique<parsers::YourParser>();
    
    // 创建应用程序实例
    app::FakeGApp app(std::move(parser));
    
    // 设置程序信息
    app.setProgramInfo("YourfakeG", "1.0.0", "你的名字");
    
    // 如果没有提供命令行参数，则交互式获取输入文件
    if (argc == 1) {
        std::cout << "YourfakeG: Convert YOUR format output to fake Gaussian format" << std::endl;
        std::cout << "Author: 你的名字" << std::endl;
        std::cout << std::endl;
        
        std::string inputFile;
        std::cout << "Please enter YOUR format output file path: ";
        std::getline(std::cin, inputFile);
        
        // 处理引号
        inputFile = string_utils::removeQuotes(inputFile);
        
        if (inputFile.empty()) {
            std::cerr << "Error: No input file provided" << std::endl;
            return 1;
        }
        
        // 构造新的argv数组
        char* newArgv[] = {argv[0], const_cast<char*>(inputFile.c_str())};
        int newArgc = 2;
        
        // 运行程序
        if (app.run(newArgc, newArgv)) {
            return 0;
        } else {
            return 1;
        }
    } else {
        // 有命令行参数，正常运行
        if (app.run(argc, argv)) {
            return 0;
        } else {
            return 1;
        }
    }
}
```

#### 4. 更新CMakeLists.txt

在 `CMakeLists.txt` 中添加你的解析器：

```cmake
# 添加你的解析器库
add_library(your_parser STATIC
    src/parsers/your_parser.cpp
)
target_link_libraries(your_parser fakeg_core)

# 添加你的可执行文件
add_executable(yourfakeg src/main/yourfake_g.cpp)
target_link_libraries(yourfakeg fakeg_app your_parser)

# 添加到安装目标
install(TARGETS yourfakeg
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(TARGETS your_parser fakeg_app DESTINATION lib)
```

### 关键解析概念

#### 数据结构

你将使用的核心数据结构：

```cpp
// 单个原子
data::Atom atom;
atom.symbol = "C";
atom.atomicNumber = 6;  // 或0表示未知（Bq虚原子）
atom.x = atom.y = atom.z = 0.0;

// 优化步骤
data::OptStep step;
step.stepNumber = 1;
step.energy = -123.456;
step.atoms = {atom1, atom2, ...};
step.converged = true;

// 频率模式
data::FreqMode mode;
mode.frequency = 1234.56;
mode.irIntensity = 12.34;
mode.irrep = "A1";
mode.displacements = {{0.1, 0.2, 0.3}, ...}; // 每个原子 [x,y,z]

// 完整的解析数据
data::ParsedData data;
data.hasOpt = true;
data.hasFreq = true;
data.optSteps = {step1, step2, ...};
data.frequencies = {mode1, mode2, ...};
data.thermoData = thermoData;
```

#### 实用工具

```cpp
// 字符串工具
using namespace fakeg::string_utils;

std::string trimmed = trim("  text  ");
std::vector<std::string> tokens = split("a,b,c", ',');
double value = toDouble("123.45", 0.0);
bool isNum = isValidNumber("1.23E-4");

// 文件行处理
LineProcessor::resetToBeginning(file);
bool found = LineProcessor::findLine(file, "SEARCH_PATTERN");
auto pos = LineProcessor::getPosition(file);
LineProcessor::setPosition(file, pos);

// 元素映射
int atomicNum = elementMap->getAtomicNumber("C");   // 返回6
int unknown = elementMap->getAtomicNumber("Xyz");   // 返回0（Bq虚原子）

// 日志记录
debugLog("调试信息");
infoLog("进度信息");
errorLog("发生错误");
```

#### 常见解析模式

```cpp
// 模式1：逐行解析
std::string line;
while (std::getline(file, line)) {
    line = string_utils::trim(line);
    
    if (string_utils::contains(line, "ENERGY")) {
        double energy = extractEnergyFromLine(line);
        // ...
    }
}

// 模式2：块解析
if (string_utils::LineProcessor::findLine(file, "BEGIN_BLOCK")) {
    while (std::getline(file, line)) {
        if (string_utils::contains(line, "END_BLOCK")) break;
        // 处理块内容
    }
}

// 模式3：结构化数据解析
std::istringstream iss(line);
std::string element;
double x, y, z;
if (iss >> element >> x >> y >> z) {
    // 成功解析原子坐标
}
```

### 测试你的解析器

1. **创建测试文件**，使用你的格式
2. **构建你的可执行文件**：
   ```bash
   ./build.sh  # 或手动cmake/make
   ```
3. **使用调试输出测试**：
   ```bash
   ./yourfakeg test_file.ext --debug
   ```
4. **验证输出格式**是否符合Gaussian期望

### 最佳实践

1. **错误处理**：始终检查文件操作和解析结果
2. **日志记录**：使用适当的日志级别（debug/info/error）
3. **虚原子**：未知元素自动成为Bq（原子序数0）
4. **文件位置**：在解析部分之间切换时重置文件位置
5. **内存效率**：增量解析，不要将整个文件加载到内存中
6. **验证**：在 `validateInput()` 中实现基本输入验证

## 核心模块参考

### 日志模块
```cpp
#include "logger/logger.h"

// 通过解析器基类可用
debugLog("详细调试信息");
infoLog("进度更新");
errorLog("错误消息");
```

### 字符串工具
```cpp
#include "string/string_utils.h"

// 文本处理
trim(), ltrim(), rtrim()
split(), toLowerCase(), toUpperCase()
startsWith(), endsWith(), contains()
replace(), replaceAll()

// 数字转换
toDouble(), toInt(), isValidNumber()
removeQuotes()  // 移除周围引号

// 文件行处理
LineProcessor::findLine()
LineProcessor::resetToBeginning()
```

### 数据结构
所有解析器都使用 `data/structures.h` 中定义的相同标准化数据结构，确保与Gaussian输出写入器的兼容性。

---

## 联系方式

作者：[Bane Dysta](https://bane-dysta.top/)