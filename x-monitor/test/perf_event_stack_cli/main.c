/*
 * @Author: CALM.WU 
 * @Date: 2021-11-12 10:13:05 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-11-12 11:38:14
 */

#include "utils/common.h"
#include "utils/x_ebpf.h"
#include "utils/resource.h"

#include <linux/perf_event.h>

#define BPF_LINKS_COUNT 2
#define SAMPLE_FREQ 50

static const char *const __default_kern_obj = "perf_event_stack_kern.o";
static sig_atomic_t      __sig_exit         = 0;
static int32_t           bpf_map_fd[2];

static void __sig_handler(int sig)
{
    __sig_exit = 1;
    fprintf(stderr, "SIGINT/SIGTERM received, exiting...\n");
}

static int32_t open_perf_event(pid_t pid, struct bpf_program *prog,
                               struct bpf_link **p_perf_event_link)
{
    int32_t pmu_fd = -1;

    struct perf_event_attr attr_type_sw = {
        .sample_freq = SAMPLE_FREQ, // 采样频率
        .freq        = 1,
        .type        = PERF_TYPE_SOFTWARE,
        .config      = PERF_COUNT_SW_CPU_CLOCK,
    };

    pmu_fd = sys_perf_event_open(&attr_type_sw, pid, -1, -1, 0);
    if (pmu_fd < 0) {
        fprintf(stderr, "sys_perf_event_open failed. %s\n", strerror(errno));
        return -1;
    }

    *p_perf_event_link = bpf_program__attach_perf_event(prog, pmu_fd);
    if (libbpf_get_error(*p_perf_event_link)) {
        fprintf(stderr, "bpf_program__attach_perf_event failed\n");
        close(pmu_fd);
        *p_perf_event_link = NULL;
        return -1;
    }

    fprintf(stderr, "open_perf_event successed\n");
    return pmu_fd;
}

int32_t main(int32_t argc, char **argv)
{
    int32_t             ret, pmu_fd;
    pid_t               pid;
    struct bpf_object  *obj;
    struct bpf_program *prog;
    struct bpf_link    *link = NULL, *perf_event_link = NULL;
    const char         *section_name, *prog_name;
    const char         *bpf_kern_o;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s %s pid\n", argv[0], __default_kern_obj);
        return -1;
    }

    bpf_kern_o = argv[1];
    pid        = strtol(argv[2], NULL, 10);

    fprintf(stderr, "Loading BPF object file: %s, target pid: %d\n", bpf_kern_o,
            pid);

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

    bpf_object__for_each_program(prog, obj)
    {
        section_name = bpf_program__section_name(prog);
        prog_name    = bpf_program__name(prog);

        fprintf(stdout, "prog: %s section: %s\n", prog_name, section_name);

        if (strcmp(prog_name, "xmonitor_bpf_collect_stack_traces") == 0 &&
            0 == strcmp(section_name, "perf_event")) {
            // 打开perf event
            if ((pmu_fd = open_perf_event(pid, prog, &perf_event_link)) < 0) {
                goto cleanup;
            }
        }

        if (0 == strcmp(section_name, "tracepoint/sched/sched_process_exit") &&
            0 == strcmp(prog_name, "__xmonitor_bpf_stack_sched_process_exit")) {
            link = bpf_program__attach(prog);

            if (libbpf_get_error(link)) {
                fprintf(stderr, "section: %s bpf_program__attach failed\n",
                        section_name);
                link = NULL;
                goto cleanup;
            }
            fprintf(stderr, "section: %s bpf program attach successed\n",
                    section_name);
        }
    }

    signal(SIGINT, __sig_handler);
    signal(SIGTERM, __sig_handler);

    while (!__sig_exit) {
        sleep(1);
    }

cleanup:
    if (link) {
        bpf_link__destroy(link);
    }

    if (perf_event_link) {
        bpf_link__destroy(perf_event_link);
    }

    bpf_object__close(obj);

    if (pmu_fd > 0) {
        close(pmu_fd);
    }

    fprintf(stdout, "%s exit\n", argv[0]);
    return 0;
}