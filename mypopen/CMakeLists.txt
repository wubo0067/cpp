cmake_minimum_required (VERSION 3.18)
project (mypopen) 

set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

# 编译输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# debug 、 release版本
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb3")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

# 版本号.
set (Mypopen_VERSION_MAJOR 1)
set (Mypopen_VERSION_MINOR 0)
 
# 配置一个头文件来传递一些CMake设置到源代码
configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_SOURCE_DIR}/config.h"
  )

# include path加上build目录 
#include_directories("${PROJECT_BINARY_DIR}")
#include_directories("${PROJECT_SOURCE_DIR}/tools")

add_subdirectory(tools)

# 项目的顶层目录
SET(CMAKE_SOURCE_DIR .)

set(MYPOPEN_FILES
        main.c
        log.c
        log.h
        popen.c
        popen.h
        )

add_executable (mypopen main.c)
target_link_libraries (mypopen tools)