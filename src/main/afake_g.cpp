#include <iostream>
#include <string>
#include "fake_g_app.h"
#include "../parsers/amesp_parser.h"
#include "../string/string_utils.h"

using namespace fakeg;

int main(int argc, char* argv[]) {
    // 创建 AMESP 解析器
    auto parser = std::make_unique<parsers::AmespParser>();
    
    // 创建应用程序实例
    app::FakeGApp app(std::move(parser));
    
    // 设置程序信息
    app.setProgramInfo("AfakeG", "1.0.0", "Bane Dysta & Claude 4.0");
    
    // 如果没有提供命令行参数，则交互式获取输入文件
    if (argc == 1) {
        std::cout << "AfakeG: Convert AMESP output to fake Gaussian format" << std::endl;
        std::cout << "Author: Bane Dysta & Claude 4.0" << std::endl;
        std::cout << std::endl;
        
        std::string inputFile;
        std::cout << "Please enter AMESP output file path: ";
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