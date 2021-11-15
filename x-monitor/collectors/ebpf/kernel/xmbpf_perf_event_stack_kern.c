/*
 * @Author: CALM.WU
 * @Date: 2021-11-11 10:35:37
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-11-12 10:41:09
 */

#include "xmbpf_helper.h"
#include <uapi/linux/bpf_perf_event.h>
#include <uapi/linux/perf_event.h>

struct process_stack_key {
    __s32 kern_stackid;
    __s32 user_stackid;
    __u32 pid;
};

struct process_stack_value {
    char  comm[TASK_COMM_LEN];
    __u32 count;
};

enum pid_filter_key {
    CTRL_FILTER_PID_1,
    CTRL_FILTER_PID_2,
    CTRL_FILTER_PID_END,
};
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 4, 14))

// 堆栈的计数器，数值代表了调用频度，也是火焰图的宽度
// key = pid
struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __type(key, struct process_stack_key);
    __type(value, struct process_stack_value);
    __uint(max_entries, roundup_pow_of_two(10240));
} process_stack_count SEC(".maps");

// 记录堆栈
struct {
    __uint(type, BPF_MAP_TYPE_STACK_TRACE);
    __uint(key_size, sizeof(__u32));
    __uint(value_size, PERF_MAX_STACK_DEPTH * sizeof(__u64)); // 堆栈的每一层
    __uint(max_entries, roundup_pow_of_two(10240));
} process_stack_map SEC(".maps");

// pid过滤器
struct {
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __type(key, __u32);
    __type(value, __u32); // pid
    __uint(max_entries, CTRL_FILTER_PID_END);
} pid_filter_map SEC(".maps");

#else

struct bpf_map_def SEC("maps") process_stack_count {
    .type = BPF_MAP_TYPE_HASH, .key_size = sizeof(struct process_stack_key),
    .value_size  = sizeof(struct process_stack_value),
    .max_entries = roundup_pow_of_two(10240),
};

struct bpf_map_def SEC("maps") process_stack_map {
    .type = BPF_MAP_TYPE_STACK_TRACE, .key_size = sizeof(__u32),
    .value_size  = PERF_MAX_STACK_DEPTH * sizeof(__u64),
    .max_entries = roundup_pow_of_two(10240),
};

struct bpf_map_def SCE("maps") pid_filter_map {
    .type = BPF_MAP_TYPE_HASH, .key_size = sizeof(__u32),
    .value_size = sizeof(__u32), .max_entries = 2,
};

#endif

#define KERN_STACKID_FLAGS (0 | BPF_F_FAST_STACK_CMP)
#define USER_STACKID_FLAGS (0 | BPF_F_FAST_STACK_CMP | BPF_F_USER_STACK)

SEC("perf_event")
__s32 xmonitor_bpf_collect_stack_traces(struct bpf_perf_event_data *ctx)
{
    __s32 pid, ret, kern_stackid, user_stackid;
    __u32 cpuid;

    struct bpf_perf_event_value perf_value_buf;
    struct process_stack_key    key = {};
    struct process_stack_value  init_value, *value;

    pid = xmonitor_get_pid();

    if (!pid) {
        return 0;
    }

    bpf_get_current_comm(init_value.comm, sizeof(init_value.comm));

    // 获取过滤的pid
    __u32  filter_pid_key = CTRL_FILTER_PID_1;
    __u32 *filter_pid_value =
        bpf_map_lookup_elem(&pid_filter_map, &filter_pid_key);
    if (filter_pid_value) {
        if (*filter_pid_value == 0) {
            return 0;
        }
        if (*filter_pid_value != pid) {
            return 0;
        }
        printk("xmonitor grab the stack for pid: %d comm: '%s'", pid, init_value.comm);
    } else {
        printk(
            "xmonitor CTRL_FILTER_PID_1 not set, So don't have to grab the stack");
        return 0;
    }

    // 得到SMP处理器ID，需要注意，所有eBPF都在禁止抢占的情况下运行，这意味着在eBPF程序的执行过程中，此ID不会改变
    cpuid = bpf_get_smp_processor_id();

    if (ctx->sample_period < 10000)
        return 0;

    // 获取堆栈信息，堆栈id放到value中，堆栈信息放到stack_map中
    kern_stackid = bpf_get_stackid(ctx, &process_stack_map, KERN_STACKID_FLAGS);
    user_stackid = bpf_get_stackid(ctx, &process_stack_map, USER_STACKID_FLAGS);

    if (kern_stackid < 0 && user_stackid < 0) {
        printk("xmonitor CPU-%d period %lld ip %llx", cpuid, ctx->sample_period,
               PT_REGS_IP(&ctx->regs));
        return 0;
    }

    if (ctx->addr != 0) {
        printk("xmonitor comm: '%s' pid: %d Address recorded on event: %llx", init_value.comm, pid, ctx->addr);
    }

    ret = bpf_perf_prog_read_value(ctx, (void *)&perf_value_buf,
                                   sizeof(struct bpf_perf_event_value));
    if (!ret)
        printk("xmonitor comm: '%s' Time Enabled: %llu, Time Running: %llu",
               init_value.comm, perf_value_buf.enabled, perf_value_buf.running);
    else
        printk("xmonitor Get Time Failed, ErrCode: %d", ret);

    key.pid          = pid;
    key.kern_stackid = kern_stackid;
    key.user_stackid = user_stackid;

    value = bpf_map_lookup_elem(&process_stack_count, &key);
    if (value) {
        // 只有一个prog更新，不用同步
        value->count++;
        //bpf_get_current_comm(&value->comm, sizeof(value->comm));
    } else {
        init_value.count = 1;
        bpf_map_update_elem(&process_stack_count, &key, &init_value,
                            BPF_NOEXIST);
    }

    return 0;
}

// SEC("tracepoint/sched/sched_process_exit")
// PROCESS_EXIT_BPF_PROG(xmonitor_bpf_stack_sched_process_exit,
//                       process_stack_count)

char _license[] SEC("license") = "GPL";
__u32 _version SEC("version")            = LINUX_VERSION_CODE;
