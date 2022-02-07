/*
 * @Author: CALM.WU
 * @Date: 2022-02-04 17:00:21
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2022-02-04 17:00:51
 */

#include <vmlinux.h>
#include "common.h"
#include <bpf/bpf_endian.h>

static __always_inline __s32 proto_is_vlan(__u16 h_proto) {
    return !!(h_proto == bpf_htons(ETH_P_8021Q) || h_proto == bpf_htons(ETH_P_8021AD));
}

static __always_inline __s32 parse_iphdr(struct hdr_cursor *nh, void *data_end,
                                         struct iphdr **iphdr) {
    struct iphdr *iph = nh->pos;
    int           hdrsize;

    if ((void *)(iph + 1) > data_end)
        return -1;

    hdrsize = iph->ihl * 4;
    /* Sanity check packet field is valid */
    if (hdrsize < sizeof(*iph))
        return -1;

    /* Variable-length IPv4 header, need to use byte-based arithmetic */
    if (nh->pos + hdrsize > data_end)
        return -1;

    nh->pos += hdrsize;
    *iphdr = iph;

    return iph->protocol;
}

static __always_inline int parse_ip6hdr(struct hdr_cursor *nh, void *data_end,
                                        struct ipv6hdr **ip6hdr) {
    struct ipv6hdr *ip6h = nh->pos;

    /* Pointer-arithmetic bounds check; pointer +1 points to after end of
     * thing being pointed to. We will be using this style in the remainder
     * of the tutorial.
     */
    if ((void *)(ip6h + 1) > data_end)
        return -1;

    nh->pos = ip6h + 1;
    *ip6hdr = ip6h;

    return ip6h->nexthdr;
}

SEC("xdp")
__s32 xdp_prog_simple(struct xdp_md *ctx) {
    // context 对象 struct xdp_md *ctx 中有包数据的 start/end 指针，可用于直接访问包数据
    void *data     = (void *)(long)ctx->data;
    void *data_end = (void *)(long)ctx->data_end;
    __s32 pkt_sz   = data_end - data;

    struct ethhdr *eth    = (struct ethhdr *)data;
    __u64          nh_off = sizeof(*eth);
    // context 对象 struct xdp_md *ctx 中有包数据的 start/end 指针，可用于直接访问包数据
    if (data + nh_off > data_end) {
        return XDP_DROP;
    }

    // 协议类型
    __u16 h_proto = eth->h_proto;
    if (proto_is_vlan(h_proto)) {
        // 判断是否是 VLAN 包
        struct vlan_hdr *vhdr;
        vhdr = (struct vlan_hdr *)(data + nh_off);  // vlan是二次打包的
        // 修改数据偏移，跳过vlan hdr，指向实际的数据包头
        nh_off += sizeof(struct vlan_hdr);
        if (data + nh_off > data_end) {
            return XDP_DROP;
        }
        // network-byte-order 这才是实际的协议，被vlan承载的
        h_proto = vhdr->h_vlan_encapsulated_proto;
    }

    __u32           ip_proto;
    struct iphdr *  iphdr;
    struct ipv6hdr *ipv6hdr;

    struct hdr_cursor nh = { .pos = data + nh_off };

    if (h_proto == bpf_htons(ETH_P_IP)) {
        // 解析ipv4协议
        ip_proto = parse_iphdr(&nh, data_end, &iphdr);
    } else if (h_proto == bpf_htons(ETH_P_IPV6)) {
        // 解析ipv6协议
        ip_proto = parse_ip6hdr(&nh, data_end, &ipv6hdr);
    } else {
        // 其他协议
        ip_proto = 0;
    }

    __bpf_printk("packet size: %d", pkt_sz);
    return XDP_PASS;
}

char _license[] SEC("license") = "GPL";