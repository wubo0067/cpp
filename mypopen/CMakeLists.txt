cmake_minimum_required (VERSION 3.18)
project (mypopen) 

set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

# 编译输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# debug 、 release版本
set(CMAKE_C_FLAGS_DEBUG "-O0 -Wall -g -ggdb3")
set(CMAKE_C_FLAGS_RELEASE "-O3 -Wall")

# 设置c编译参数
set(CMAKE_C_FLAGS "-fPIC")

# 设置c标准 cmake3.18只支持c11
set(CMAKE_C_STANDARD 11)

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
include_directories("${PROJECT_SOURCE_DIR}")

add_subdirectory(tools)

# 项目的顶层目录
SET(CMAKE_SOURCE_DIR .)


set(PLUGINSD_PLUGIN_FILES
        plugins.d/plugins_d.c
        plugins.d/plugins_d.h
        )

add_executable (mypopen main.c
                        ${PLUGINSD_PLUGIN_FILES}
                        )
                        
target_link_libraries (mypopen tools)