/*
 * @Author: CALM.WU 
 * @Date: 2021-11-02 14:01:24 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-11-03 19:34:29
 */

#include <linux/ptrace.h>
#include <linux/version.h>
#include <linux/threads.h>
#include <uapi/linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <trace_common.h>

#include "xmonitor_bpf_helper.h"

struct cachestat_key {
    __u32 pid;                 // 进程ID
    __u32 uid;                 // 用户ID
    char  comm[TASK_COMM_LEN]; // 进程名
};

struct cachestat_value {
    __u64 add_to_page_cache_lru;
    __u64 ip_add_to_page_cache; // IP寄存器的值
    __u64 mark_page_accessed;
    __u64 ip_mark_page_accessed; // IP寄存器的值
    __u64 account_page_dirtied;
    __u64 ip_account_page_dirtied; // IP寄存器的值
    __u64 mark_buffer_dirty;
    __u64 ip_mark_buffer_dirty; // IP寄存器的值
};

#define CACHE_STATE_MAX_SIZE 1024

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 4, 14))

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, CACHE_STATE_MAX_SIZE);
    __type(key, struct cachestat_key);
    __type(value, struct cachestat_value);
} cachestat_map SEC(".maps");

#else

struct bpf_map_def SEC("maps") cachestat_map = {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0))
    .type        = BPF_MAP_TYPE_HASH,
#else
    .type = BPF_MAP_TYPE_PERCPU_HASH,
#endif
    .key_size    = sizeof(struct cachestat_key),
    .value_size  = sizeof(struct cachestat_value),
    .max_entries = CACHE_STATE_MAX_SIZE,
};

#endif

#define FILTER_SYMBOL_COUNT 4

// static char __filter_pcomm[FILTER_PCOMM_COUNT][16] = {
//     "ksmtuned",
// };

static const struct ftiler_symbol {
    char  name[32];
    __s32 length;
} symbols[FILTER_SYMBOL_COUNT] = {
    {
        .name   = "ksmtuned",
        .length = 8,
    },
    {
        .name = "pmie_check",
        .length = 10,
    },
    {
        .name = "polkitd",
        .length = 7,
    },  
    {
        .name = "pmdadm",
        .length = 6,
    },      
};

static __s32 filter_out_symbol(char comm[TASK_COMM_LEN])
{
    __s32 i, j, find;

    for (i = 0; i < FILTER_SYMBOL_COUNT; i++) {
        find = 1;
        for (j = 0; j < symbols[i].length && j < TASK_COMM_LEN; j++) {
            if (comm[j] != symbols[i].name[j]) {
                find = 0;
                break;
            }
        }
        if (find) {
            printk("filter out '%s'\n", comm);
            break;
        }
    }
    return find;
}

/************************************************************************************
 *
 *                                   Probe Section
 *
 ***********************************************************************************/
SEC("kprobe/add_to_page_cache_lru")
__s32 xmonitor_add_to_page_cache_lru(struct pt_regs *ctx)
{
    __s32                   ret = 0;
    struct cachestat_key    key = {};
    struct cachestat_value *fill;

    // 得到应用程序名
    bpf_get_current_comm(&key.comm, sizeof(key.comm));

    if(filter_out_symbol(key.comm)) {
        return 0;
    }

    // 得到进程ID
    key.pid = (pid_t)bpf_get_current_pid_tgid();
    // 得到用户ID
    key.uid = (uid_t)bpf_get_current_uid_gid();

    fill = bpf_map_lookup_elem(&cachestat_map, &key);
    if (fill) {
        xmonitor_update_u64(&fill->add_to_page_cache_lru, 1);
        printk("add_to_page_cache_lru pcomm: '%s' pid: %d value: %lu", key.comm,
               key.pid, fill->add_to_page_cache_lru);
        //bpf_map_update_elem(&cachestat_map, &key, value, BPF_EXIST);
    } else {
        struct cachestat_value init_value = {
            .add_to_page_cache_lru = 1,
        };
        init_value.ip_add_to_page_cache = PT_REGS_IP(ctx);

        ret =
            bpf_map_update_elem(&cachestat_map, &key, &init_value, BPF_NOEXIST);
        if (0 == ret) {
            printk(
                "add_to_page_cache_lru add new pcomm: '%s' pid: %d successed",
                key.comm, key.pid);
        } else {
            printk(
                "add_to_page_cache_lru add new pcomm: '%s' pid: %d failed error: %d",
                key.comm, key.pid, ret);
        }
    }
    return 0;
}

SEC("kprobe/mark_page_accessed")
__s32 xmonitor_mark_page_accessed(struct pt_regs *ctx)
{
    __s32                   ret = 0;
    struct cachestat_key    key = {};
    struct cachestat_value *fill;

    // 得到应用程序名
    bpf_get_current_comm(&key.comm, sizeof(key.comm));

    if(filter_out_symbol(key.comm)) {
        return 0;
    }

    // 得到进程ID
    key.pid = (pid_t)bpf_get_current_pid_tgid();
    // 得到用户ID
    key.uid = (uid_t)bpf_get_current_uid_gid();

    fill = bpf_map_lookup_elem(&cachestat_map, &key);
    if (fill) {
        xmonitor_update_u64(&fill->mark_page_accessed, 1);
        printk("mark_page_accessed pcomm: '%s' pid: %d value: %lu", key.comm,
               key.pid, fill->mark_page_accessed);
        //bpf_map_update_elem(&cachestat_map, &key, value, BPF_EXIST);
    } else {
        struct cachestat_value init_value = {
            .mark_page_accessed = 1,
        };
        init_value.ip_mark_page_accessed = PT_REGS_IP(ctx);

        ret =
            bpf_map_update_elem(&cachestat_map, &key, &init_value, BPF_NOEXIST);
        if (0 == ret) {
            printk("mark_page_accessed add new pcomm: '%s' pid: %d successed",
                   key.comm, key.pid);
        } else {
            printk(
                "mark_page_accessed add new pcomm: '%s' pid: %d failed error: %d",
                key.comm, key.pid, ret);
        }
    }
    return 0;
}

SEC("kprobe/account_page_dirtied")
__s32 xmonitor_account_page_dirtied(struct pt_regs *ctx)
{
    __s32                   ret = 0;
    struct cachestat_key    key = {};
    struct cachestat_value *fill;

    // 得到应用程序名
    bpf_get_current_comm(&key.comm, sizeof(key.comm));

    if(filter_out_symbol(key.comm)) {
        return 0;
    }

    // 得到进程ID
    key.pid = (pid_t)bpf_get_current_pid_tgid();
    // 得到用户ID
    key.uid = (uid_t)bpf_get_current_uid_gid();

    fill = bpf_map_lookup_elem(&cachestat_map, &key);
    if (fill) {
        xmonitor_update_u64(&fill->account_page_dirtied, 1);
        printk("account_page_dirtied pcomm: '%s' pid: %d value: %lu", key.comm,
               key.pid, fill->account_page_dirtied);
        //bpf_map_update_elem(&cachestat_map, &key, value, BPF_EXIST);
    } else {
        struct cachestat_value init_value = {
            .account_page_dirtied = 1,
        };
        init_value.ip_account_page_dirtied = PT_REGS_IP(ctx);

        ret =
            bpf_map_update_elem(&cachestat_map, &key, &init_value, BPF_NOEXIST);
        if (0 == ret) {
            printk("account_page_dirtied add new pcomm: '%s' pid: %d successed",
                   key.comm, key.pid);
        } else {
            printk(
                "account_page_dirtied add new pcomm: '%s' pid: %d failed error: %d",
                key.comm, key.pid, ret);
        }
    }
    return 0;
}

SEC("kprobe/mark_buffer_dirty")
__s32 xmonitor_mark_buffer_dirty(struct pt_regs *ctx)
{
    __s32                   ret = 0;
    struct cachestat_key    key = {};
    struct cachestat_value *fill;

    // 得到应用程序名
    bpf_get_current_comm(&key.comm, sizeof(key.comm));

    if(filter_out_symbol(key.comm)) {
        return 0;
    }

    // 得到进程ID
    key.pid = (pid_t)bpf_get_current_pid_tgid();
    // 得到用户ID
    key.uid = (uid_t)bpf_get_current_uid_gid();

    fill = bpf_map_lookup_elem(&cachestat_map, &key);
    if (fill) {
        xmonitor_update_u64(&fill->mark_buffer_dirty, 1);
        printk("mark_buffer_dirty pcomm: '%s' pid: %d value: %lu", key.comm,
               key.pid, fill->mark_buffer_dirty);
        //bpf_map_update_elem(&cachestat_map, &key, value, BPF_EXIST);
    } else {
        struct cachestat_value init_value = {
            .mark_buffer_dirty = 1,
        };
        init_value.ip_mark_buffer_dirty = PT_REGS_IP(ctx);

        ret =
            bpf_map_update_elem(&cachestat_map, &key, &init_value, BPF_NOEXIST);
        if (0 == ret) {
            printk("mark_buffer_dirty add new pcomm: '%s' pid: %d successed",
                   key.comm, key.pid);
        } else {
            printk(
                "mark_buffer_dirty add new pcomm: '%s' pid: %d failed error: %d",
                key.comm, key.pid, ret);
        }
    }
    return 0;
}

#define PROG(foo)                                                                              \
    __s32 foo(void *ctx)                                                                       \
    {                                                                                          \
        struct cachestat_key key = {};                                                         \
                                                                                               \
        bpf_get_current_comm(&key.comm, sizeof(key.comm));                                     \
        key.pid   = (pid_t)bpf_get_current_pid_tgid();                                         \
        key.uid   = (uid_t)bpf_get_current_uid_gid();                                          \
        __s32 ret = bpf_map_delete_elem(&cachestat_map, &key);                                 \
        if (0 == ret) {                                                                        \
            printk(                                                                            \
                "pcomm: '%s' pid: %d uid: %d exit. remove element from cachestat_map.",        \
                key.comm, key.pid, key.uid);                                                   \
        } else {                                                                               \
            printk(                                                                            \
                "pcomm: '%s' pid: %d exit. remove element from cachestat_map failed. ret: %d", \
                key.comm, key.pid, ret);                                                       \
        }                                                                                      \
        return 0;                                                                              \
    }

// SEC("tracepoint/sched/sched_process_exit")
// __s32 xmonitor_sched_process_exit(void *ctx)
// {
//     struct cachestat_key key = {};

//     bpf_get_current_comm(&key.comm, sizeof(key.comm));
//     key.pid = (pid_t)bpf_get_current_pid_tgid();
//     key.uid = (uid_t)bpf_get_current_uid_gid();

//     __s32 ret = bpf_map_delete_elem(&cachestat_map, &key);
//     if (0 == ret) {
//         printk(
//             "pcomm: '%s' pid: %d uid: %d exit. remove element from cachestat_map.",
//             key.comm, key.pid, key.uid);
//     } else {
//         printk(
//             "pcomm: '%s' pid: %d exit. remove element from cachestat_map failed. ret: %d",
//             key.comm, key.pid, ret);
//     }

//     return 0;
// }

SEC("tracepoint/sched/sched_process_exit")
PROG(xmonitor_sched_process_exit)

SEC("tracepoint/sched/sched_process_free")
PROG(xmonitor_sched_process_free)

char           _license[] SEC("license") = "GPL";
__u32 _version SEC("version")            = LINUX_VERSION_CODE;