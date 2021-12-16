/*
 * @Author: CALM.WU 
 * @Date: 2021-11-03 11:26:22 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-11-03 16:48:18
 */

#pragma once

#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <linux/compiler.h>

#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <linux/bpf.h>
#include <linux/ptrace.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <trace_helpers.h>
#include <perf-sys.h>

extern int32_t bpf_printf(enum libbpf_print_level level, const char *fmt,
                          va_list args);

extern const char *bpf_get_ksym_name(uint64_t addr);

extern int32_t open_raw_sock(const char *iface);

// struct perf_event_attr;

// static inline int sys_perf_event_open(struct perf_event_attr *attr, pid_t pid,
//                                       int cpu, int group_fd,
//                                       unsigned long flags)
// {
//     return syscall(__NR_perf_event_open, attr, pid, cpu, group_fd, flags);
// }

#ifdef __cplusplus
}
#endif