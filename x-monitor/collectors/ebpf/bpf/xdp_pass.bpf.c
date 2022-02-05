/*
 * @Author: CALM.WU
 * @Date: 2022-02-04 17:00:21
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2022-02-04 17:00:51
 */

#include <linux/bpf.h>
#include "common.h"

SEC("xdp")
__s32 xdp_prog_simple(struct xdp_md *ctx) {
    void *data     = (void *)(long)ctx->data;
    void *data_end = (void *)(long)ctx->data_end;
    __s32 pkt_sz   = data_end - data;

    __bpf_printk("packet size: %d", pkt_sz);
    return XDP_PASS;
}

char _license[] SEC("license") = "GPL";