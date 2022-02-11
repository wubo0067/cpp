/*
 * @Author: CALM.WU
 * @Date: 2022-02-11 10:36:28
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2022-02-11 18:01:16
 */
#include <argp.h>

#include "utils/common.h"
#include "utils/compiler.h"
#include "utils/debug.h"
#include "utils/resource.h"
#include "utils/consts.h"
#include "utils/x_ebpf.h"

#include <bpf/libbpf.h>
#include <linux/if_link.h>
#include "xdp_pass.skel.h"

struct args {
    int32_t itf_index;
    bool    verbose;
} env = {
    .itf_index = -1,  // 所有的网卡
    .verbose   = true,
};

static uint32_t __xdp_flags = XDP_FLAGS_UPDATE_IF_NOEXIST;

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

    __xdp_flags |= XDP_FLAGS_SKB_MODE;

    libbpf_set_strict_mode(LIBBPF_STRICT_ALL);
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
        debug("BPF object opened");
    }

    // 加载
    ret = xdp_pass_bpf__load(obj);
    if (unlikely(0 != ret)) {
        fprintf(stderr, "failed to load BPF object: %d\n", ret);
        goto cleanup;
    } else {
        debug("BPF object loaded");
    }

    // debug("xdp_prog_simple name: %s sec_name: %s", obj->progs.xdp_prog_simple->name,
    //       obj->progs.xdp_prog_simple->sec_name);

    int32_t map_fd = bpf_map__fd(obj->maps.ipproto_rx_cnt_map);
    debug("bpf map ipproto_rx_cnt_map fd:%d", map_fd);

    int32_t prog_fd = bpf_program__fd(obj->progs.xdp_prog_simple);
    debug("bpf prog xdp_prog_simple fd:%d", map_fd);

    // 附加 它是可选的，你可以通过直接使用 libbpf API 获得更多控制）；
    // ret = xdp_pass_bpf__attach(obj);
    ret = bpf_xdp_attach(prog_fd, env.itf_index, __xdp_flags, NULL);
    if (ret) {
        fprintf(stderr, "failed to attach BPF programs. ret: %d err: %s\n", ret, strerror(errno));
        goto cleanup;
    } else {
        debug("BPF programs attached");
    }

    sleep(1);

    bpf_xdp_detach(env.itf_index, __xdp_flags, NULL);

cleanup:
    xdp_pass_bpf__destroy(obj);

    debug("%s exit", argv[0]);
    debugClose();
    return 0;
}