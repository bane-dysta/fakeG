#include <iostream>
#include "fake_g_app.h"
#include "../parsers/amesp_parser.h"

using namespace fakeg;

int main(int argc, char* argv[]) {
    // 创建 AMESP 解析器
    auto parser = std::make_unique<parsers::AmespParser>();
    
    // 创建应用程序实例
    app::FakeGApp app(std::move(parser));
    
    // 设置程序信息
    app.setProgramInfo("AfakeG", "1.0.0", "Bane Dysta & Claude 4.0");
    
    // 运行程序
    if (app.run(argc, argv)) {
        return 0;
    } else {
        return 1;
    }
} 