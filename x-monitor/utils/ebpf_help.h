/*
 * @Author: CALM.WU 
 * @Date: 2021-11-03 11:26:22 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-11-03 16:48:18
 */

#pragma once

#include <stdint.h>

#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <linux/bpf.h>
#include <linux/ptrace.h>
#include <trace_helpers.h>

extern int32_t bpf_printf(enum libbpf_print_level level, const char *fmt,
                          va_list args);
extern const char *bpf_get_ksym_name(uint64_t addr);