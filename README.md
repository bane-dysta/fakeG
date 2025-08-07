# FakeG - 模块化量子化学输出转换工具

## 项目简介

FakeG是一个模块化的C++项目，用于将各种量子化学软件的输出文件转换为Gaussian格式的伪输出文件。该项目采用了现代C++设计，具有良好的可扩展性和可维护性。

## 项目特点

- **模块化设计**：代码组织清晰，各模块职责分明
- **可扩展架构**：通过接口和工厂模式，轻松添加新的解析器
- **配置分离**：支持YAML配置文件，代码与配置分离
- **日志系统**：完整的日志管理，支持多级别输出
- **跨平台**：支持Linux、Windows和macOS

## 项目结构

```
FakeG/
├── src/                    # 源代码目录
│   ├── data/              # 数据结构模块
│   │   ├── structures.h   # 数据结构定义
│   │   └── structures.cpp # 数据结构实现
│   ├── io/                # IO模块
│   │   ├── file_reader.h/cpp    # 文件读取
│   │   └── gaussian_writer.h/cpp # Gaussian格式输出
│   ├── logger/            # 日志模块
│   │   ├── logger.h
│   │   └── logger.cpp
│   ├── string/            # 字符串工具模块
│   │   ├── string_utils.h
│   │   └── string_utils.cpp
│   ├── parsers/           # 解析器模块
│   │   ├── parser_interface.h/cpp  # 解析器接口
│   │   ├── amesp_parser.h/cpp      # AMESP解析器
│   │   └── bdf_parser.h/cpp        # BDF解析器
│   └── main/              # 主程序模块
│       ├── fake_g_app.h/cpp        # 应用程序类
│       ├── afake_g.cpp             # AfakeG主程序
│       └── bfake_g.cpp             # BfakeG主程序
├── config/                # 配置文件目录
│   ├── afakeg.yaml       # AfakeG配置
│   └── bfakeg.yaml       # BfakeG配置
├── CMakeLists.txt        # CMake构建文件
└── README.md             # 项目文档
```

## 编译安装

### 系统要求

- CMake 3.16+
- C++20兼容的编译器（GCC 10+, Clang 12+, MSVC 2019+）

### 编译步骤

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译
make -j$(nproc)

# 安装（可选）
sudo make install
```

### Debug版本编译

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

## 使用方法

### AfakeG (AMESP格式转换)

```bash
# 基本用法
./afakeg input.out

# 开启调试模式
./afakeg input.out --debug

# 指定输出文件
./afakeg input.out -o output.log

# 显示帮助
./afakeg --help
```

### BfakeG (BDF格式转换)

```bash
# 基本用法
./bfakeg input.out

# 开启调试模式  
./bfakeg input.out --debug
```

## 扩展开发

### 添加新解析器

1. **创建解析器类**：继承`ParserInterface`

```cpp
// src/parsers/your_parser.h
#pragma once
#include "parser_interface.h"

class YourParser : public ParserInterface {
public:
    bool parse(io::FileReader& reader, data::ParsedData& data) override;
    std::string getParserName() const override { return "your_format"; }
    std::string getParserVersion() const override { return "1.0"; }
    std::string getFileExtension() const override { return ".your_ext"; }
    
private:
    // 你的解析方法
};
```

2. **实现解析逻辑**：

```cpp
// src/parsers/your_parser.cpp
#include "your_parser.h"

bool YourParser::parse(io::FileReader& reader, data::ParsedData& data) {
    auto& file = reader.getStream();
    
    // 解析几何结构
    // 解析能量信息
    // 解析频率数据
    // ...
    
    return true;
}
```

3. **注册解析器**：

```cpp
// src/main/your_fake_g.cpp
#include "your_parser.h"

int main(int argc, char* argv[]) {
    // 注册你的解析器
    parsers::ParserFactory::registerParser<YourParser>("your_format", ".your_ext");
    
    app::FakeGApp app;
    app.setProgramInfo("YourFakeG", "1.0", "Your Name");
    return app.run(argc, argv) ? 0 : 1;
}
```

4. **更新CMakeLists.txt**：

```cmake
# 添加解析器库
set(YOUR_PARSER_SOURCES
    src/parsers/your_parser.cpp
)

add_library(your_parser ${YOUR_PARSER_SOURCES})
target_link_libraries(your_parser fakeg_core)

# 添加可执行文件
add_executable(yourfakeg src/main/your_fake_g.cpp)
target_link_libraries(yourfakeg fakeg_app your_parser)
```

### 配置文件支持

创建配置文件`config/yourfakeg.yaml`：

```yaml
program:
  name: "YourFakeG"
  version: "1.0"
  author: "Your Name"

input:
  extensions: [".your_ext"]
  encoding: "utf8"

output:
  suffix: "_fake"
  extension: ".log"

parser:
  name: "your_format"
  options:
    parse_optimization: true
    parse_frequencies: true
```

## 核心模块说明

### 数据结构模块 (data/)

定义了所有解析器和输出器使用的通用数据结构：
- `Atom`: 原子信息
- `OptStep`: 优化步骤
- `FreqMode`: 频率模式
- `ThermoData`: 热力学数据
- `ParsedData`: 解析结果容器

### IO模块 (io/)

- **FileReader**: 文件读取，支持编码检测和转换
- **GaussianWriter**: Gaussian格式输出，标准化格式

### Logger模块 (logger/)

提供分级日志系统：
```cpp
#include "logger/logger.h"

// 使用全局logger
LOG_DEBUG("调试信息");
LOG_INFO("普通信息");
LOG_WARNING("警告信息"); 
LOG_ERROR("错误信息");

// 或使用logger实例
logger::Logger myLogger(true); // 启用debug
myLogger.debug("调试信息");
myLogger.info("普通信息");
```

### 字符串工具模块 (string/)

提供常用字符串处理功能：
```cpp
#include "string/string_utils.h"

using namespace fakeg::string_utils;

std::string trimmed = trim("  hello world  ");
std::vector<std::string> tokens = split("a,b,c", ',');
bool isNum = isNumber("123.45");
double value = toDouble("123.45", 0.0);
```

## 项目设计理念

### 1. 模块化架构
每个模块职责单一，接口清晰，便于测试和维护。

### 2. 接口导向
使用抽象接口和工厂模式，支持运行时选择不同的实现。

### 3. 配置分离
核心逻辑与配置参数分离，支持不同环境下的灵活配置。

### 4. 现代C++
充分利用C++20特性，提供类型安全和性能优化。

### 5. 错误处理
完善的错误处理机制，提供详细的错误信息和调试支持。

## 贡献指南

1. Fork项目
2. 创建特性分支：`git checkout -b feature/your-feature`
3. 提交更改：`git commit -am 'Add your feature'`
4. 推送分支：`git push origin feature/your-feature`  
5. 提交Pull Request

## 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 联系方式

- 作者：Bane Dysta
- 项目主页：[项目仓库地址]
- 问题反馈：[Issue页面地址]

---

**注意**：项目还在积极开发中，API可能会发生变化。建议在生产环境使用前充分测试。 