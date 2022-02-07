/*
 * @Author: CALM.WU
 * @Date: 2022-02-04 14:29:03
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2022-02-04 16:22:10
 */

//#include <linux/bpf.h> // uapi这个头文件和vmlinux.h不兼容啊，类型重复定义
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

#ifndef ETH_P_8021Q
#define ETH_P_8021Q 0x8100 /* 802.1Q VLAN Extended Header  */
#endif

#ifndef ETH_P_8021AD
#define ETH_P_8021AD 0x88A8 /* 802.1ad Service VLAN */
#endif

#ifndef ETH_P_IP
#define ETH_P_IP 0x0800 /* Internet Protocol packet */
#endif

#ifndef ETH_P_IPV6
#define ETH_P_IPV6 0x86DD /* Internet Protocol Version 6 packet */
#endif

struct hdr_cursor {
    void *pos;
};

static __always_inline void xmonitor_update_u64(__u64 *res, __u64 value) {
    __sync_fetch_and_add(res, value);
    if ((0xFFFFFFFFFFFFFFFF - *res) <= value) {
        *res = value;
    }
}

static __always_inline __u32 xmonitor_get_pid() {
    return bpf_get_current_pid_tgid() >> 32;
}

static __always_inline __s32 xmonitor_get_tid() {
    return bpf_get_current_pid_tgid() & 0xFFFFFFFF;
}