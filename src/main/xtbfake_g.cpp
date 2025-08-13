#include <iostream>
#include <memory>
#include "fake_g_app.h"
#include "../parsers/xtb_parser.h"
#include "../string/string_utils.h"

using namespace fakeg;

int main(int argc, char* argv[]) {
    try {
        if (argc == 1) {
            // 交互模式
            std::cout << "XtbfakeG: Convert XTB Gaussian format output to fake Gaussian format" << std::endl;
            std::cout << "Author: Bane Dysta & Claude 4.0" << std::endl;
            std::cout << std::endl;
            
            std::string inputFile;
            std::cout << "Please enter XTB Gaussian format output file path: ";
            std::getline(std::cin, inputFile);
            inputFile = string_utils::removeQuotes(inputFile);
            
            if (inputFile.empty()) {
                std::cerr << "Error: No input file specified" << std::endl;
                return 1;
            }
            
            // 创建解析器和应用程序
            auto parser = std::make_unique<parsers::XtbParser>();
            app::FakeGApp app(std::move(parser));
            
            // 构造新的argv数组
            char* newArgv[] = {const_cast<char*>("xtbfakeg"), const_cast<char*>(inputFile.c_str())};
            int newArgc = 2;
            
            // 运行转换
            return app.run(newArgc, newArgv) ? 0 : 1;
            
        } else {
            // 命令行模式
            auto parser = std::make_unique<parsers::XtbParser>();
            app::FakeGApp app(std::move(parser));
            
            return app.run(argc, argv) ? 0 : 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
} 