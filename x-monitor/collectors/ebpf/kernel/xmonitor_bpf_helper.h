/*
 * @Author: CALM.WU 
 * @Date: 2021-11-02 15:08:24 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-11-03 17:27:20
 */

#pragma once

#include <linux/sched.h>
#include <linux/version.h>

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