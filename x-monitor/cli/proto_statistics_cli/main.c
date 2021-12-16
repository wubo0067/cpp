/*
 * @Author: CALM.WU 
 * @Date: 2021-12-16 14:05:09 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-12-16 14:16:59
 */

#include "utils/common.h"
#include "utils/x_ebpf.h"
#include "utils/resource.h"
#include "utils/debug.h"

static const char *const __ebpf_kern_obj =
    "../collectors/ebpf/kernel/xmbpf_proto_statistics_kern.5.12.o";

static sig_atomic_t __sig_exit = 0;

static void __sig_handler(int sig) {
    __sig_exit = 1;
    debug("SIGINT/SIGTERM received, exiting...\n");
}

int32_t main(int argc, char const *argv[]) {
    int32_t ret = 0;

    struct bpf_object *obj = NULL;
    int32_t            proto_countmap_fd, prog_fd;
    int32_t            sock;

    if (argc != 3) {
        fprintf(
            stderr,
            "Usage: proto_statistics_cli xmbpf_proto_statistics_kern.o eth0\n");
        return -1;
    }

    debugLevel = 9;
    debugFile = fdopen(STDOUT_FILENO, "w");

    const char *bpf_kern_o = argv[1];
    const char *iface = argv[2];

    libbpf_set_print(bpf_printf);

    ret = bump_memlock_rlimit();
    if (ret) {
        fprintf(stderr, "failed to increase memlock rlimit: %s\n",
                strerror(errno));
        return -1;
    }

    if (0 != bpf_prog_load(bpf_kern_o, BPF_PROG_TYPE_SOCKET_FILTER, &obj,
                           &prog_fd)) {
        fprintf(stderr, "failed to load BPF program %s: %s\n", __ebpf_kern_obj,
                strerror(errno));
        return -1;
    }

    proto_countmap_fd = bpf_object__find_map_fd_by_name(obj, "proto_countmap");
    if (proto_countmap_fd < 0) {
        fprintf(stderr,
                "ERROR: finding a map 'proto_countmap' in obj file failed\n");
        goto cleanup;
    }

    // 在网络接口上打开原始套接字
    sock = open_raw_sock(iface);
    if (sock < 0) {
        goto cleanup;
    }

    // 套接字绑定bpf程序
    if (0 != setsockopt(sock, SOL_SOCKET, SO_ATTACH_BPF, &prog_fd,
                        sizeof(prog_fd))) {
        fprintf(stderr, "setsockopt SO_ATTACH_BPF failed: %s\n",
                strerror(errno));
        goto cleanup;
    }

    signal(SIGINT, __sig_handler);
    signal(SIGTERM, __sig_handler);

    uint32_t tcp_cnt, udp_cnt, icmp_cnt;
    int32_t  proto_key;

    debug("Starting to protocol statistics ...\n");

    while (!__sig_exit) {
        proto_key = IPPROTO_TCP;
        bpf_map_lookup_elem(proto_countmap_fd, &proto_key, &tcp_cnt);

        proto_key = IPPROTO_UDP;
        bpf_map_lookup_elem(proto_countmap_fd, &proto_key, &udp_cnt);

        proto_key = IPPROTO_ICMP;
        bpf_map_lookup_elem(proto_countmap_fd, &proto_key, &icmp_cnt);

        debug("TCP: %u, UDP: %u, ICMP: %u\n", tcp_cnt, udp_cnt, icmp_cnt);
        sleep(1);
    }

cleanup:
    bpf_object__close(obj);

    debug("%s exit", argv[0]);
    debugClose();
    return 0;
}
