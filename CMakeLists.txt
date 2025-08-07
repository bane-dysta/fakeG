cmake_minimum_required(VERSION 3.16)
project(FakeG VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# 调试模式设置
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
endif()

# 包含目录
include_directories(src)

# 创建通用库
set(FAKEG_CORE_SOURCES
    src/data/structures.cpp
    src/logger/logger.cpp
    src/string/string_utils.cpp
    src/io/file_reader.cpp
    src/io/gaussian_writer.cpp
    src/parsers/parser_interface.cpp
    src/main/fake_g_app.cpp
)

add_library(fakeg_core ${FAKEG_CORE_SOURCES})

# 解析器库
set(AMESP_PARSER_SOURCES
    src/parsers/amesp_parser.cpp
)

add_library(amesp_parser ${AMESP_PARSER_SOURCES})
target_link_libraries(amesp_parser fakeg_core)

set(BDF_PARSER_SOURCES
    src/parsers/bdf_parser.cpp
)

add_library(bdf_parser ${BDF_PARSER_SOURCES})
target_link_libraries(bdf_parser fakeg_core)

# 主应用程序库
set(FAKEG_APP_SOURCES
    src/main/fake_g_app.cpp
)

add_library(fakeg_app ${FAKEG_APP_SOURCES})
target_link_libraries(fakeg_app fakeg_core)

# AfakeG可执行文件
add_executable(afakeg src/main/afake_g.cpp)
target_link_libraries(afakeg fakeg_app amesp_parser)

# 暂时注释掉BfakeG
# 创建BfakeG可执行文件
add_executable(bfakeg src/main/bfake_g.cpp)
target_link_libraries(bfakeg fakeg_core bdf_parser)

# 安装配置
install(TARGETS afakeg bfakeg DESTINATION bin)
install(TARGETS fakeg_core DESTINATION lib)
install(TARGETS amesp_parser DESTINATION lib)
install(TARGETS bdf_parser DESTINATION lib)
install(DIRECTORY config/ DESTINATION share/fakeg/config)
install(FILES README.md DESTINATION share/doc/fakeg)

# 创建配置文件安装
# install(FILES config/afakeg.yaml config/bfakeg.yaml
#     DESTINATION share/fakeg/config
#     OPTIONAL
# )

# 创建文档安装
# install(FILES README.md
#     DESTINATION share/doc/fakeg
#     OPTIONAL
# )

# CPack配置（打包）
set(CPACK_PACKAGE_NAME "FakeG")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "FakeG - Quantum Chemistry Output Converter")
set(CPACK_PACKAGE_VENDOR "FakeG Project")
include(CPack) 