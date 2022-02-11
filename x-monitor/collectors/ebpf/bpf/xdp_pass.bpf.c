/*
 * @Author: CALM.WU
 * @Date: 2022-02-04 17:00:21
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2022-02-11 16:25:04
 */

// https://mp.weixin.qq.com/s/fX4HyWdY9AalQLpj5zhoYw

#include <vmlinux.h>
#include <bpf/bpf_endian.h>
#include "common.h"
#include "parsing_helpers.h"

// ip协议包数量统计
struct {
    __uint(type, BPF_MAP_TYPE_PERCPU_ARRAY);
    __type(key, __u32);
    __type(value, __u64);
    __uint(max_entries, 256);
} ipproto_rx_cnt_map SEC(".maps");

SEC("xdp") __s32 xdp_prog_simple(struct xdp_md *ctx) {
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
        // 返回ipv4包承载的协议类型
        ip_proto = parse_iphdr(&nh, data_end, &iphdr);
    } else if (h_proto == bpf_htons(ETH_P_IPV6)) {
        // 返回ipv6包承载的协议类型
        ip_proto = parse_ip6hdr(&nh, data_end, &ipv6hdr);
    } else {
        // 其他协议
        ip_proto = 0;
    }

    __u64 *rx_cnt = bpf_map_lookup_elem(&ipproto_rx_cnt_map, &ip_proto);
    if (rx_cnt) {
        *rx_cnt += 1;
    } else {
        __u64 init_value = 1;
        bpf_map_update_elem(&ipproto_rx_cnt_map, &ip_proto, &init_value, BPF_NOEXIST);
    }

    return XDP_PASS;
}

char _license[] SEC("license") = "GPL";