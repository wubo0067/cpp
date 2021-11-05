/*
 * @Author: CALM.WU 
 * @Date: 2021-11-02 15:08:24 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-11-05 15:18:22
 */

#pragma once

#include <linux/ptrace.h>
#include <linux/version.h>
#include <linux/threads.h>
#include <linux/sched.h>
#include <uapi/linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>
#include <trace_common.h>

#define printk(fmt, ...)                                                       \
    ({                                                                         \
        char ____fmt[] = fmt;                                                  \
        bpf_trace_printk(____fmt, sizeof(____fmt), ##__VA_ARGS__);             \
    })

static __always_inline void xmonitor_update_u64(__u64 *res, __u64 value)
{
    __sync_fetch_and_add(res, value);
    if ((0xFFFFFFFFFFFFFFFF - *res) <= value) {
        *res = value;
    }
}

static __always_inline __s32 xmonitor_get_pid()
{
    return bpf_get_current_pid_tgid() >> 32;
}

static __always_inline __s32 xmonitor_get_tid() {
    return bpf_get_current_pid_tgid() & 0xFFFFFFFF;
}

// static __always_inline __s32 xmonitor_get_comm(char (*p)[TASK_COMM_LEN]) {
//     struct task_struct *task = (struct task_struct *)bpf_get_current_task();
//     return bpf_core_read_str(&p, TASK_COMM_LEN, &task->comm);
// }