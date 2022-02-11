/*
 * @Author: CALM.WU
 * @Date: 2022-02-11 10:36:28
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2022-02-11 15:25:21
 */
#include <argp.h>

#include "utils/common.h"
#include "utils/compiler.h"
#include "utils/debug.h"
#include "utils/resource.h"
#include "utils/consts.h"
#include "utils/x_ebpf.h"

#include <bpf/libbpf.h>
#include "xdp_pass.skel.h"

struct args {
    int32_t itf_index;
    bool    verbose;
} env = {
    .itf_index = -1,  // 所有的网卡
    .verbose   = true,
};

static const struct argp_option __opts[] = {
    { "itf", 'i', "-1", OPTION_ARG_OPTIONAL, "Interface name", 0 },
    { "verbose", 'v', NULL, 0, "Verbose debug output", 0 },
    { NULL },
};

static error_t parse_arg(int key, char *arg, struct argp_state *state) {
    switch (key) {
    case 'v':
        env.verbose = true;
        break;
    case 'i':
        env.itf_index = if_nametoindex(arg);
        if (unlikely(!env.itf_index)) {
            fprintf(stderr, "invalid interface name: %s, err: %s\n", arg, strerror(errno));
            exit(EXIT_FAILURE);
        }
        debug("ift_index:%d, itf name:%s", env.itf_index, arg);
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

int32_t main(int32_t argc, char **argv) {
    int32_t ret = 0;

    debugLevel = 9;
    debugFile  = fdopen(STDOUT_FILENO, "w");

    static const struct argp argp = {
        .options = __opts,
        .parser  = parse_arg,
        .doc     = "./xdp_pass_test --itf=ens160 -v",
    };

    ret = argp_parse(&argp, argc, argv, 0, NULL, NULL);
    if (unlikely(ret)) {
        return -1;
    }

    // Libbpf 日志
    if (env.verbose) {
        libbpf_set_print(bpf_printf);
    }

    ret = bump_memlock_rlimit();
    if (ret) {
        fprintf(stderr, "failed to increase memlock rlimit: %s\n", strerror(errno));
        return -1;
    }

    // 打开
    struct xdp_pass_bpf *obj = xdp_pass_bpf__open();
    if (unlikely(!obj)) {
        fprintf(stderr, "failed to open and/or load BPF object\n");
        return -1;
    } else {
        debug("BPF object loaded");
    }

    // 加载
    ret = xdp_pass_bpf__load(obj);
    if (unlikely(0 != ret)) {
        fprintf(stderr, "failed to load BPF object: %d\n", ret);
        goto cleanup;
    } else {
        debug("BPF object loaded");
    }

cleanup:
    xdp_pass_bpf__destroy(obj);

    debug("%s exit", argv[0]);
    debugClose();
    return 0;
}