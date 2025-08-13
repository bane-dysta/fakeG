cmake_minimum_required(VERSION 3.16)
project(FakeG VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 静态链接选项
option(STATIC_LINKING "Enable static linking for better portability" OFF)
option(FULL_STATIC "Enable full static linking (including glibc)" OFF)
option(WINDOWS_BUILD "Enable Windows cross-compilation using mingw-w64" OFF)

# Windows交叉编译设置
if(WINDOWS_BUILD)
    message(STATUS "启用Windows交叉编译")
    set(CMAKE_SYSTEM_NAME Windows)
    set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
    set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
    set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)
    set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    
    # Windows静态链接设置
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
    
    # 设置可执行文件后缀
    set(CMAKE_EXECUTABLE_SUFFIX ".exe")
endif()

# 编译选项
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
    # Windows编译时的特殊设置
    if(WINDOWS_BUILD)
        add_compile_options(-DWIN32 -D_WIN32_WINNT=0x0601)
    endif()
endif()

# 调试模式设置
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
endif()

# Linux静态链接设置（不适用于Windows交叉编译）
if(NOT WINDOWS_BUILD)
    if(FULL_STATIC)
        message(STATUS "启用完全静态链接 (包括glibc)")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
        set(BUILD_SHARED_LIBS OFF)
    elseif(STATIC_LINKING)
        message(STATUS "启用部分静态链接 (libgcc, libstdc++)")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    endif()
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

# XYZ Parser library
set(XYZ_PARSER_SOURCES
    src/parsers/xyz_parser.cpp
)

add_library(xyz_parser ${XYZ_PARSER_SOURCES})
target_link_libraries(xyz_parser fakeg_core)

# 创建xtb_parser库
add_library(xtb_parser STATIC
    src/parsers/xtb_parser.cpp
)
target_link_libraries(xtb_parser fakeg_core)

# 创建应用程序框架库
add_library(fakeg_app STATIC
    src/main/fake_g_app.cpp
)
target_link_libraries(fakeg_app fakeg_core)

# AfakeG可执行文件
add_executable(afakeg src/main/afake_g.cpp)
target_link_libraries(afakeg fakeg_app amesp_parser)

# BfakeG可执行文件
add_executable(bfakeg src/main/bfake_g.cpp)
target_link_libraries(bfakeg fakeg_core bdf_parser)

# XfakeG executable
add_executable(xfakeg src/main/xfake_g.cpp)
target_link_libraries(xfakeg fakeg_app xyz_parser)

# 创建xtbfakeg可执行文件
add_executable(xtbfakeg src/main/xtbfake_g.cpp)
target_link_libraries(xtbfakeg fakeg_app xtb_parser)

# 静态链接时的特殊处理（Linux）
if((FULL_STATIC OR STATIC_LINKING) AND NOT WINDOWS_BUILD)
    # 确保使用静态库
    set_target_properties(fakeg_core PROPERTIES 
        LINK_SEARCH_START_STATIC 1
        LINK_SEARCH_END_STATIC 1)
    set_target_properties(amesp_parser PROPERTIES 
        LINK_SEARCH_START_STATIC 1
        LINK_SEARCH_END_STATIC 1)
    set_target_properties(bdf_parser PROPERTIES 
        LINK_SEARCH_START_STATIC 1
        LINK_SEARCH_END_STATIC 1)
    set_target_properties(fakeg_app PROPERTIES 
        LINK_SEARCH_START_STATIC 1
        LINK_SEARCH_END_STATIC 1)
    set_target_properties(xyz_parser PROPERTIES 
        LINK_SEARCH_START_STATIC 1
        LINK_SEARCH_END_STATIC 1)
        
    # 可执行文件的静态链接设置
    set_target_properties(afakeg PROPERTIES 
        LINK_SEARCH_START_STATIC 1
        LINK_SEARCH_END_STATIC 1)
    set_target_properties(bfakeg PROPERTIES 
        LINK_SEARCH_START_STATIC 1
        LINK_SEARCH_END_STATIC 1)
    set_target_properties(xfakeg PROPERTIES 
        LINK_SEARCH_START_STATIC 1
        LINK_SEARCH_END_STATIC 1)
endif()

# Windows特殊处理
if(WINDOWS_BUILD)
    # 为Windows可执行文件设置属性
    set_target_properties(afakeg PROPERTIES
        WIN32_EXECUTABLE FALSE
        SUFFIX ".exe")
    set_target_properties(bfakeg PROPERTIES
        WIN32_EXECUTABLE FALSE
        SUFFIX ".exe")
    set_target_properties(xfakeg PROPERTIES
        WIN32_EXECUTABLE FALSE
        SUFFIX ".exe")
endif()

# 安装目标
install(TARGETS afakeg bfakeg xfakeg xtbfakeg
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(TARGETS fakeg_core amesp_parser bdf_parser xyz_parser xtb_parser fakeg_app DESTINATION lib)
install(DIRECTORY config/ DESTINATION share/fakeg/config)
install(FILES README.md DESTINATION share/doc/fakeg)

# CPack配置（打包）
set(CPACK_PACKAGE_NAME "FakeG")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "FakeG - Quantum Chemistry Output Converter")
set(CPACK_PACKAGE_VENDOR "FakeG Project")
include(CPack)

# 显示编译信息
message(STATUS "=== FakeG 编译配置 ===")
message(STATUS "C++ 标准: ${CMAKE_CXX_STANDARD}")
message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")
if(WINDOWS_BUILD)
    message(STATUS "目标平台: Windows (交叉编译)")
    message(STATUS "编译器: ${CMAKE_CXX_COMPILER}")
else()
    message(STATUS "目标平台: Linux")
    if(FULL_STATIC)
        message(STATUS "链接方式: 完全静态链接")
    elseif(STATIC_LINKING)
        message(STATUS "链接方式: 部分静态链接")
    else()
        message(STATUS "链接方式: 动态链接")
    endif()
endif()
message(STATUS "========================") 