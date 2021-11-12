# 目录下所有的源代码文件并将列表存储在cachestat_cli_srcs中
AUX_SOURCE_DIRECTORY(. perf_event_stack_cli_srcs)

set(EBPF_HELPER_OBJS
	${KERNEL_DIR}/tools/testing/selftests/bpf/trace_helpers.o)

include_directories("${EBPF_USER_INCLUDE_DIRS}")

link_directories("${KERNEL_DIR}/tools/lib/bpf")

# 加上trace_helpers.o文件到链接队列中
add_executable(perf_event_stack_cli ${perf_event_stack_cli_srcs} ${EBPF_HELPER_OBJS})

add_custom_target(gen_perf_event_stack_skel 
				COMMAND /usr/sbin/bpftool gen skeleton ${PROJECT_SOURCE_DIR}/collectors/ebpf/kernel/perf_event_stack_kern.o > ${PROJECT_SOURCE_DIR}/test/cachestat_cli/perf_event_stack_skel.h
				VERBATIM
				)

target_link_libraries(perf_event_stack_cli xmonitor-utils libbpf.a elf pthread z m) 
add_dependencies(perf_event_stack_cli gen_perf_event_stack_skel)  