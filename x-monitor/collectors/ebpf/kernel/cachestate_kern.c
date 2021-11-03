/*
 * @Author: CALM.WU 
 * @Date: 2021-11-02 14:01:24 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-11-03 15:00:39
 */

#include <linux/ptrace.h>
#include <linux/version.h>
#include <linux/threads.h>
#include <uapi/linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <trace_common.h>

#include "xmonitor_bpf_helper.h"

struct cachestate_key_t {
	__u64 ip; // IP寄存器的值
    __u32 pid; // 进程ID
    __u32 uid; // 用户ID
    char comm[ TASK_COMM_LEN ]; // 进程名
};

#define CACHE_STATE_MAX_SIZE (PID_MAX_DEFAULT * 4)

struct bpf_map_def SEC("maps") cachestate_map = {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0))    
    .type = BPF_MAP_TYPE_HASH,
#else
    .type = BPF_MAP_TYPE_PERCPU_HASH,
#endif
    .key_size = sizeof(struct cachestate_key_t),
    .value_size = sizeof(__u64),
    .max_entries = CACHE_STATE_MAX_SIZE,
};

/************************************************************************************
 *
 *                                   Probe Section
 *
 ***********************************************************************************/
SEC("kprobe/add_to_page_cache_lru")
__s32 xmonitor_add_to_page_cache_lru(struct pt_regs* ctx) {
    struct cachestate_key_t key = {};
    __u64 init_val = 1;
    // 得到应用程序名
    bpf_get_current_comm( &key.comm, sizeof( key.comm ) );
    // 得到进程ID
    key.pid = (pid_t)bpf_get_current_pid_tgid();
    // 得到用户ID
    key.uid = (uid_t)bpf_get_current_uid_gid();
    // 得到IP寄存器的值
    key.ip = PT_REGS_IP(ctx);

    __u64 *value = bpf_map_lookup_elem( &cachestate_map, &key );
    if ( value ) {
        xmonitor_update_u64(value, 1);
    } else {
        bpf_map_update_elem( &cachestate_map, &key, &init_val, BPF_NOEXIST );
    }
    return 0;
}

SEC("kprobe/mark_page_accessed")
__s32 xmonitor_mark_page_accessed(struct pt_regs* ctx) {
    struct cachestate_key_t key = {};
    __u64 init_val = 1;
    // 得到应用程序名
    bpf_get_current_comm( &key.comm, sizeof( key.comm ) );
    // 得到进程ID
    key.pid = (pid_t)bpf_get_current_pid_tgid();
    // 得到用户ID
    key.uid = (uid_t)bpf_get_current_uid_gid();
    // 得到IP寄存器的值
    key.ip = PT_REGS_IP(ctx);

    __u64 *value = bpf_map_lookup_elem( &cachestate_map, &key );
    if ( value ) {
        xmonitor_update_u64(value, 1);
    } else {
        bpf_map_update_elem( &cachestate_map, &key, &init_val, BPF_NOEXIST );
    }
    return 0;
}

SEC("kprobe/account_page_dirtied")
__s32 xmonitor_account_page_dirtied(struct pt_regs* ctx) {
    struct cachestate_key_t key = {};
    __u64 init_val = 1;
    // 得到应用程序名
    bpf_get_current_comm( &key.comm, sizeof( key.comm ) );
    // 得到进程ID
    key.pid = (pid_t)bpf_get_current_pid_tgid();
    // 得到用户ID
    key.uid = (uid_t)bpf_get_current_uid_gid();
    // 得到IP寄存器的值
    key.ip = PT_REGS_IP(ctx);

    __u64 *value = bpf_map_lookup_elem( &cachestate_map, &key );
    if ( value ) {
        xmonitor_update_u64(value, 1);
    } else {
        bpf_map_update_elem( &cachestate_map, &key, &init_val, BPF_NOEXIST );
    }
    return 0;
}

SEC("kprobe/mark_buffer_dirty")
__s32 xmonitor_mark_buffer_dirty(struct pt_regs* ctx) {
    struct cachestate_key_t key = {};
    __u64 init_val = 1;
    // 得到应用程序名
    bpf_get_current_comm( &key.comm, sizeof( key.comm ) );
    // 得到进程ID
    key.pid = (pid_t)bpf_get_current_pid_tgid();
    // 得到用户ID
    key.uid = (uid_t)bpf_get_current_uid_gid();
    // 得到IP寄存器的值
    key.ip = PT_REGS_IP(ctx);

    __u64 *value = bpf_map_lookup_elem( &cachestate_map, &key );
    if ( value ) {
        xmonitor_update_u64(value, 1);
    } else {
        bpf_map_update_elem( &cachestate_map, &key, &init_val, BPF_NOEXIST );
    }
    return 0;
}