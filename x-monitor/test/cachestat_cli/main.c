/*
 * @Author: CALM.WU 
 * @Date: 2021-11-03 11:23:12 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-11-03 14:58:40
 */

#include "utils/common.h"
#include "utils/ebpf_help.h"
#include "utils/resource.h"

const char *const cachestat_kern_obj =
    "../collectors/ebpf/kernel/ebpf_cachestate_kern.5.12.o";

int32_t main(int32_t argc, char **argv)
{
    int32_t             map_fd, ret, j = 0;
    struct bpf_object  *obj;
    struct bpf_program *prog;
    struct bpf_link    *links[4]; // 这个是SEC数量
    const char         *section;
    char                symbol[256];

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

    obj = bpf_object__open_file(cachestat_kern_obj, NULL);
    if (libbpf_get_error(obj)) {
        fprintf(stderr, "ERROR: opening BPF object file '%s' failed\n",
                cachestat_kern_obj);
        ret = -2;
        goto cleanup;
    }

    /* load BPF program */
    if (bpf_object__load(obj)) {
        fprintf(stderr, "ERROR: loading BPF object file failed\n");
        ret = -2;
        goto cleanup;
    }

    // find map
    map_fd = bpf_object__find_map_fd_by_name(obj, "cachestate_map");
    if (map_fd < 0) {
        fprintf(stderr,
                "ERROR: finding a map 'cachestate_map' in obj file failed\n");
        goto cleanup;
    }

    bpf_object__for_each_program(prog, obj)
    {
        section = bpf_program__section_name(prog);
        fprintf(stdout, "[%d] section: %s\n", j, section);
        if (sscanf(section, "kprobe/%s", symbol) != 1) {
            fprintf(stderr, "ERROR: invalid section name: %s\n", section);
            continue;
        }

        /* Attach prog only when symbol exists */
        if (ksym_get_addr(symbol)) {
            //prog->log_level = 1;
            links[j]        = bpf_program__attach(prog);
            if (libbpf_get_error(links[j])) {
                fprintf(stderr, "[%d] section: %s bpf_program__attach failed\n", j, section);
                links[j] = NULL;
                goto cleanup;
            }
            fprintf(stderr, "[%d] section: %s bpf program attach successed\n", j, section);
            j++;
        }
    }

    while(1) {
        sleep(1);
    }

cleanup:
    for (j--; j >= 0; j--)
        bpf_link__destroy(links[j]);

    bpf_object__close(obj);

    fprintf(stdout, "cachestat_cli exit\n");
    return ret;
}