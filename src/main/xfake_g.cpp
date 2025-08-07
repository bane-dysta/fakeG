#include <iostream>
#include "fake_g_app.h"
#include "../parsers/xyz_parser.h"
#include "../string/string_utils.h"

using namespace fakeg;

int main(int argc, char* argv[]) {
    // 创建XYZ解析器实例
    auto parser = std::make_unique<parsers::XyzParser>();
    
    // 创建注入解析器的应用程序
    app::FakeGApp app(std::move(parser));
    
    // 设置程序信息
    app.setProgramInfo("XfakeG", "1.0.0", "Bane Dysta & Claude 4.0");
    
    // 如果没有提供参数，进入交互模式
    if (argc < 2) {
        std::cout << "XfakeG: Convert XYZ/TRJ trajectory to fake Gaussian format" << std::endl;
        std::cout << "Author: Bane Dysta & Claude 4.0" << std::endl;
        std::cout << std::endl;
        std::cout << "Please enter XYZ/TRJ trajectory file path: ";
        
        std::string inputFile;
        std::getline(std::cin, inputFile);
        
        // 处理引号
        inputFile = string_utils::removeQuotes(inputFile);
        
        if (inputFile.empty()) {
            std::cerr << "Error: No input file provided" << std::endl;
            return 1;
        }
        
        // 创建临时参数数组
        char* tempArgv[] = {argv[0], const_cast<char*>(inputFile.c_str())};
        int tempArgc = 2;
        
        // 运行应用程序
        if (app.run(tempArgc, tempArgv)) {
            return 0;
        } else {
            return 1;
        }
    }
    
    // 运行应用程序
    if (app.run(argc, argv)) {
        return 0;
    } else {
        return 1;
    }
} 