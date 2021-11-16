/*
 * @Author: CALM.WU 
 * @Date: 2021-11-12 10:13:05 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-11-12 17:23:50
 */

#include "utils/common.h"
#include "utils/x_ebpf.h"
#include "utils/resource.h"
#include "utils/debug.h"

#include <linux/perf_event.h>

#define SAMPLE_FREQ 100 // 100hz
#define MAX_CPU_NR 128

enum ctrl_filter_key {
    CTRL_FILTER,
    CTRL_FILTER_END,
};

#define __FILTER_CONTENT_LEN 128

struct ctrl_filter_value {
    uint32_t filter_pid;
    char  filter_content[__FILTER_CONTENT_LEN];
};

static const char *const __default_kern_obj = "perf_event_stack_kern.o";
static sig_atomic_t      __sig_exit         = 0;
static int32_t           bpf_map_fd[3];

static void __sig_handler(int sig)
{
    __sig_exit = 1;
    debug("SIGINT/SIGTERM received, exiting...");
}

static int32_t open_and_attach_perf_event(struct bpf_program *prog,
                                          int32_t             nr_cpus,
                                          struct bpf_link   **perf_event_links)
{
    int32_t pmu_fd = -1;

    struct perf_event_attr attr = {
        .sample_period = SAMPLE_FREQ, // 采样频率
        .freq          = 1,           //
        .type          = PERF_TYPE_SOFTWARE,
        .config        = PERF_COUNT_SW_CPU_CLOCK,
        .inherit       = 0,
    };

    for (int32_t i = 0; i < nr_cpus; i++) {
        // 这将测量指定CPU上的所有进程/线程
        pmu_fd = sys_perf_event_open(&attr, -1, i, -1, 0);
        if (pmu_fd < 0) {
            fprintf(stderr, "sys_perf_event_open failed. %s\n",
                    strerror(errno));
            return -1;
        } else {
            debug("on cpu: %d sys_perf_event_open successed, pmu_fd: %d", i,
                  pmu_fd);
        }

        perf_event_links[i] = bpf_program__attach_perf_event(prog, pmu_fd);
        if (libbpf_get_error(perf_event_links[i])) {
            fprintf(stderr, "failed to attach perf event on cpu: %d\n", i);
            close(pmu_fd);
            perf_event_links[i] = NULL;
            return -1;
        } else {
            debug("prog: '%s' attach perf event on cpu: %d successed",
                  bpf_program__name(prog), i);
        }
    }

    return 0;
}

int32_t main(int32_t argc, char **argv)
{
    int32_t             ret, pmu_fd;
    pid_t               filter_pid = -1;
    struct bpf_object  *obj;
    struct bpf_program *prog;
    struct bpf_link   **perf_event_links;
    const char         *bpf_kern_o;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s %s pid\n", argv[0], __default_kern_obj);
        return -1;
    }

    debugLevel = 9;
    debugFile  = fdopen(STDOUT_FILENO, "w");

    bpf_kern_o = argv[1];
    filter_pid = strtol(argv[2], NULL, 10);

    if (filter_pid == 0 || errno == EINVAL || errno == ERANGE) {
        debug("filter pid %s is invalid", argv[2]);
        return -1;
    }

    debug("Loading BPF object file: %s, filter pid: %d\n", bpf_kern_o,
          filter_pid);

    if (load_kallsyms()) {
        fprintf(stderr, "failed to process /proc/kallsyms\n");
        return -1;
    }

    libbpf_set_print(bpf_printf);

    ret = bump_memlock_rlimit();
    if (ret) {
        fprintf(stderr, "failed to increase memlock rlimit: %s\n",
                strerror(errno));
        return -1;
    }

    obj = bpf_object__open_file(bpf_kern_o, NULL);
    if (libbpf_get_error(obj)) {
        fprintf(stderr, "ERROR: opening BPF object file '%s' failed\n",
                bpf_kern_o);
        ret = -2;
        goto cleanup;
    }

    /* load BPF program */
    if (bpf_object__load(obj)) {
        fprintf(stderr, "ERROR: loading BPF object file failed\n");
        ret = -2;
        goto cleanup;
    }

    // 宽度
    bpf_map_fd[0] = bpf_object__find_map_fd_by_name(obj, "process_stack_count");
    if (bpf_map_fd[0] < 0) {
        fprintf(
            stderr,
            "ERROR: finding a map 'process_stack_count' in obj file failed\n");
        goto cleanup;
    }

    bpf_map_fd[1] = bpf_object__find_map_fd_by_name(obj, "process_stack_map");
    if (bpf_map_fd[1] < 0) {
        fprintf(
            stderr,
            "ERROR: finding a map 'process_stack_map' in obj file failed\n");
        goto cleanup;
    }

    bpf_map_fd[2] = bpf_object__find_map_fd_by_name(obj, "ctrl_filter_map");
    if (bpf_map_fd[1] < 0) {
        fprintf(stderr,
                "ERROR: finding a map 'ctrl_filter_map' in obj file failed\n");
        goto cleanup;
    }

    // 设置过滤的pid
    enum ctrl_filter_key filter_key = CTRL_FILTER;
    struct ctrl_filter_value filter_value = {
        .filter_pid = filter_pid,
    };
    strncpy(filter_value.filter_content, __default_kern_obj, strlen(__default_kern_obj));

    ret = bpf_map_update_elem(bpf_map_fd[2], &filter_key, &filter_value,
                              BPF_ANY);
    if (0 != ret) {
        fprintf(stderr, "ERROR: bpf_map_update_elem filter value failed, ret: %d\n", ret);
        goto cleanup;
    } 

    // 打开perf event
    prog = bpf_object__find_program_by_name(
        obj, "xmonitor_bpf_collect_stack_traces");
    if (!prog) {
        fprintf(
            stderr,
            "finding prog: 'xmonitor_bpf_collect_stack_traces' in obj file failed\n");
        goto cleanup;
    }

    int nr_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    perf_event_links =
        (struct bpf_link **)calloc(nr_cpus, sizeof(struct bpf_link *));
    debug("nr cpu count: %d", nr_cpus);

    if ((pmu_fd = open_and_attach_perf_event(prog, nr_cpus, perf_event_links)) <
        0) {
        goto cleanup;
    } else {
        debug("open perf event with prog: '%s' successed",
              bpf_program__name(prog));
    }

    signal(SIGINT, __sig_handler);
    signal(SIGTERM, __sig_handler);

    while (!__sig_exit) {
        sleep(1);
    }

cleanup:
    if (perf_event_links) {
        for (int32_t i = 0; i < nr_cpus; i++) {
            if (perf_event_links[i]) {
                bpf_link__destroy(perf_event_links[i]);
            }
        }
        free(perf_event_links);
    }

    bpf_object__close(obj);

    debug("%s exit", argv[0]);
    debugClose();
    return 0;
}