# 目录下所有的源代码文件并将列表存储在XMONITOR_UTILS_SRCS中
AUX_SOURCE_DIRECTORY(. XMONITOR_UTILS_SRCS)

#会追加到父CMakeLists.txt的include变量中
include_directories("${KERNEL_DIR}/usr/include"
                    "${KERNEL_DIR}/tools/include"
                    "${KERNEL_DIR}/tools/perf"
                    "${KERNEL_DIR}/tools/lib"
                    "${KERNEL_DIR}/tools/testing/selftests/bpf")

#设置静态库输出目录                   
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/libs)
add_library(xmonitor-utils ${XMONITOR_UTILS_SRCS})