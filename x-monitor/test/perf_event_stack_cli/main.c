/*
 * @Author: CALM.WU 
 * @Date: 2021-11-12 10:13:05 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-11-12 11:10:24
 */

#include "utils/common.h"
#include "utils/x_ebpf.h"
#include "utils/resource.h"

#define BPF_LINKS_COUNT 2

static const char *const __default_kern_obj = "perf_event_stack_kern.o";
static sig_atomic_t      __sig_exit         = 0;
static int32_t           bpf_map_fd[2];

static void __sig_handler(int sig)
{
    __sig_exit = 1;
    fprintf(stderr, "SIGINT/SIGTERM received, exiting...\n");
}

int32_t main(int32_t argc, char **argv)
{
    int32_t             j = 0;
    int32_t             ret;
    struct bpf_object  *obj;
    struct bpf_program *prog;
    struct bpf_link    *link, *perf_event_link;
    const char         *section;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s %s\n", argv[0], __default_kern_obj);
        return -1;
    }

    const char *bpf_kern_o = argv[1];

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
        section = bpf_program__section_name(prog);
        fprintf(stdout, "[%d] section: %s\n", j, section);

        if (0 == strcmp(section, "tracepoint/sched/sched_process_exit")) {
            link = bpf_program__attach(prog);

            if (libbpf_get_error(link)) {
                fprintf(stderr, "section: %s bpf_program__attach failed\n",
                        section);
                link = NULL;
                goto cleanup;
            }
            fprintf(stderr, "section: %s bpf program attach successed\n",
                    section);
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

    fprintf(stdout, "%s exit\n", argv[0]);
    return 0;
}