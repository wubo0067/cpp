/*
 * @Author: CALM.WU 
 * @Date: 2021-11-03 11:23:12 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-11-03 18:04:19
 */

#include "utils/common.h"
#include "utils/ebpf_help.h"
#include "utils/resource.h"

const char *const cachestat_kern_obj =
    "../collectors/ebpf/kernel/ebpf_cachestate_kern.5.12.o";

struct cachestate_key_t {
    uint64_t ip;       // IP寄存器的值
    uint32_t pid;      // 进程ID
    uint32_t uid;      // 用户ID
    char     comm[16]; // 进程名
};

static sig_atomic_t __sig_exit = 0;

static void sig_handler(int sig)
{
    __sig_exit = 1;
    fprintf(stderr, "SIGINT/SIGTERM received, exiting...\n");
}

int32_t main(int32_t argc, char **argv)
{
    int32_t             map_fd, ret, j = 0, result = 0;
    struct bpf_object  *obj;
    struct bpf_program *prog;
    struct bpf_link    *links[4]; // 这个是SEC数量
    const char         *section;
    char                symbol[256];

    struct cachestate_key_t key = {}, next_key;
    uint64_t                cachestate_map_value;

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
            links[j] = bpf_program__attach(prog);
            if (libbpf_get_error(links[j])) {
                fprintf(stderr, "[%d] section: %s bpf_program__attach failed\n",
                        j, section);
                links[j] = NULL;
                goto cleanup;
            }
            fprintf(stderr, "[%d] section: %s bpf program attach successed\n",
                    j, section);
            j++;
        }
    }

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    while (!__sig_exit) {
        while (bpf_map_get_next_key(map_fd, &key, &next_key) == 0) {
            if (__sig_exit) {
                fprintf(stdout, "--------------\n");
                break;
            }
            key = next_key;

            result =
                bpf_map_lookup_elem(map_fd, &next_key, &cachestate_map_value);
            if (0 == result) {
                fprintf(stdout, "%-16s %-6s %-6s %-19s %-16s %s\n", "PCOMM", "PID", "UID",
                        "ADDR", "SYMBOL", "COUNT");
                fprintf(stderr, "'%-16s' %-6d %-6d 0x%-17lx %-16s %5ld\n",
                        next_key.comm, next_key.pid, next_key.uid, next_key.ip,
                        bpf_get_ksym_name(next_key.ip), cachestate_map_value);
            } else {
                fprintf(stderr, "ERROR: bpf_map_lookup_elem fail to get entry value of Key: '%s'\n", key.comm);
            }
        }
    }

    fprintf(stdout, "kprobing funcs exit\n");

cleanup:
    for (j--; j >= 0; j--)
        bpf_link__destroy(links[j]);

    bpf_object__close(obj);

    fprintf(stdout, "cachestat_cli exit\n");
    return ret;
}