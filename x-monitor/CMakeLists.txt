cmake_minimum_required (VERSION 3.18)
project (x-monitor) 

set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

# 编译输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# debug 、 release版本
set(CMAKE_C_FLAGS_DEBUG "-O0 -Wall -g -ggdb3")
set(CMAKE_C_FLAGS_RELEASE "-O3 -Wall")

# 设置c编译参数
set(CMAKE_C_FLAGS "-fPIC -Wno-unknown-pragmas -Wextra")

# 设置c标准 cmake3.18只支持c11
set(CMAKE_C_STANDARD 11)

# 版本号.
set (XMonitor_VERSION_MAJOR 1)
set (XMonitor_VERSION_MINOR 0)

# 依赖项
set(EXTRA_DIR "${PROJECT_SOURCE_DIR}/extra")
set(KERNEL_DIR "/usr/src/linux-5.12.9")
 
# 配置一个头文件来传递一些CMake设置到源代码
configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_SOURCE_DIR}/config.h"
  )

# ebpf user include路径
set(EBPF_USER_INCLUDE_DIRS "${KERNEL_DIR}/usr/include"
                           "${KERNEL_DIR}/tools/include"
                           "${KERNEL_DIR}/tools/perf"
                           "${KERNEL_DIR}/tools/lib"
                           "${KERNEL_DIR}/tools/testing/selftests/bpf")

# 头文件include目录
include_directories("${PROJECT_SOURCE_DIR}" 
                    "${EXTRA_DIR}/include")

# 第三方静态库链接路径
link_directories("${PROJECT_SOURCE_DIR}/extra/libs")

# 第三方库
set(EXTRA_LIBS "libjson-c.a" "libconfig.a" "libzlog.a" "libcollectc.a")

# 自定义宏
add_definitions(
        -DCONFIG_FILENAME="/etc/x-monitor/x-monitor.cfg"
        -DLOG_DIR="/usr/log/x-monitor"
)

# 项目子目录
add_subdirectory(utils)
add_subdirectory(test/construct_init)
add_subdirectory(test/cachestat_cli)
add_subdirectory(test/perf_event_stack_cli)

# 子目录源代码
set(PLUGINSD_FILES
        plugins.d/plugins_d.c
        plugins.d/plugins_d.h
        )

set(APPCONFIG_FILES
        appconfig/appconfig.h
        appconfig/appconfig.c
        )

# 编译的执行程序
add_executable(x-monitor main.c
                        ${PLUGINSD_FILES}
                        ${APPCONFIG_FILES}
                        )
                        
target_link_libraries(x-monitor xmonitor-utils ${EXTRA_LIBS} pthread z m)