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

// bin/xdp_libbpf_test --itf=ens160 -v

enum xdp_action_type {
    XDP_ACTION_ATTACH = 0,
    XDP_ACTION_DETACH,
};

struct args {
    int32_t              itf_index;
    enum xdp_action_type action_type;
    uint32_t             xdp_flags;
    bool                 verbose;
} env = {
    .itf_index = -1,  // 所有的网卡
    .xdp_flags = XDP_FLAGS_UPDATE_IF_NOEXIST,
    .verbose   = true,
};

static uint32_t __prog_id;

static const char *__optstr = "i:sfv";

static const struct option __opts[] = {
    { "itf", required_argument, NULL, 'i' }, { "type", required_argument, NULL, 't' },
    { "skb", no_argument, NULL, 's' },       { "force_load", no_argument, NULL, 'f' },
    { "verbose", no_argument, NULL, 'v' },   { 0, 0, 0, 0 },
};

static void usage(const char *prog) {
    fprintf(stderr,
            "usage: %s [OPTS] --itf=IFNAME\n\n"
            "OPTS:\n"
            "    -s    use skb-mode\n"
            "    -f    force loading prog\n"
            "    -v    verbose\n",
            prog);
}

static void sig_handler(int sig) {
    __u32 curr_prog_id = 0;

    if (bpf_get_link_xdp_id(env.itf_index, &curr_prog_id, env.xdp_flags)) {
        debug("bpf_get_link_xdp_id failed\n");
        exit(1);
    }
    if (__prog_id == curr_prog_id)
        bpf_set_link_xdp_fd(env.itf_index, -1, env.xdp_flags);
    else if (!curr_prog_id)
        debug("couldn't find a prog id on a given interface\n");
    else
        debug("program on interface changed, not removing\n");
    return;
}

int32_t main(int32_t argc, char **argv) {
    int32_t ret = 0;
    int32_t opt;

    debugLevel = 9;
    debugFile  = fdopen(STDOUT_FILENO, "w");

    while ((opt = getopt_long(argc, argv, __optstr, __opts, NULL)) != -1) {
        switch (opt) {
        case 'v':
            env.verbose = true;
            break;
        case 'i':
            env.itf_index = if_nametoindex(optarg);
            if (unlikely(!env.itf_index)) {
                fprintf(stderr, "invalid interface name: %s, err: %s\n", optarg, strerror(errno));
                exit(EXIT_FAILURE);
            }
            debug("ift_index:%d, itf_name:%s", env.itf_index, optarg);
            break;
        case 's':
            env.xdp_flags |= XDP_FLAGS_SKB_MODE;
            debug("xdp flags set skb mode");
            break;
        case 'f':
            env.xdp_flags &= ~XDP_FLAGS_UPDATE_IF_NOEXIST;
            debug("xdp flags force load");
            break;
        case 't':
            if (!strcmp(optarg, "attach")) {
                env.action_type = XDP_ACTION_ATTACH;
            } else if (!strcmp(optarg, "detach")) {
                env.action_type = XDP_ACTION_DETACH;
            } else {
                fprintf(stderr, "invalid action type: %s\n", optarg);
                exit(EXIT_FAILURE);
            }
            break;
        default:
            usage(basename(argv[0]));
            return 1;
        }
    }

    if (!(env.xdp_flags & XDP_FLAGS_SKB_MODE)) {
        env.xdp_flags |= XDP_FLAGS_DRV_MODE;
    }

    // if (optind == argc) {
    //     debug("optind:%d, argc:%d", optind, argc);
    //     usage(basename(argv[0]));
    //     return 1;
    // }

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

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    // 附加 它是可选的，你可以通过直接使用 libbpf API 获得更多控制）；
    // ret = xdp_pass_bpf__attach(obj);
    // ret = bpf_xdp_attach(prog_fd, env.itf_index, __xdp_flags, NULL);
    // 使用bpf_set_link_xdp_fd执行attach成功，和bpf_xdp_attach差异在于old_prog_fd这个参数
    ret = bpf_set_link_xdp_fd(env.itf_index, prog_fd, env.xdp_flags);
    if (ret < 0) {
        fprintf(stderr, "link set xdp fd failed. ret: %d err: %s\n", ret, strerror(errno));
        goto cleanup;
    } else {
        debug("BPF programs attached");
    }

    struct bpf_prog_info info     = {};
    uint32_t             info_len = sizeof(info);

    ret = bpf_obj_get_info_by_fd(prog_fd, &info, &info_len);
    if (ret) {
        fprintf(stderr, "can't get prog info: %s\n", strerror(errno));
    }
    __prog_id = info.id;
    debug("prog id: %d", __prog_id);

    sleep(1000);

    // bpf_xdp_detach(env.itf_index, __xdp_flags, NULL);
    debug("xdp detach");

cleanup:
    xdp_pass_bpf__destroy(obj);

    debug("%s exit", argv[0]);
    debugClose();
    return 0;
}