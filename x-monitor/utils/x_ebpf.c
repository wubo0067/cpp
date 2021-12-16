/*
 * @Author: CALM.WU 
 * @Date: 2021-11-03 11:37:51 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-11-03 16:42:01
 */

#include "x_ebpf.h"
#include "common.h"

static const char *__ksym_empty_name = "";

int32_t bpf_printf(enum libbpf_print_level level, const char *fmt, va_list args)
{
    // if ( level == LIBBPF_DEBUG && !g_env.verbose ) {
    // 	return 0;
    // }
    char out_fmt[128] = { 0 };
    sprintf(out_fmt, "level:{%d} %s", level, fmt);
    // vfprintf适合参数可变列表传递
    return vfprintf(stderr, out_fmt, args);
}

const char *bpf_get_ksym_name(uint64_t addr)
{
    struct ksym *sym;

    if (addr == 0)
        return __ksym_empty_name;

    sym = ksym_search(addr);
    if (!sym)
        return __ksym_empty_name;

    return sym->name;
}

int32_t open_raw_sock(const char *iface) {
    struct sockaddr_ll sll; 
    int32_t sock;

    sock = socket(PF_PACKET, SOCK_RAW | SOCK_NONBLOCK | SOCK_CLOEXEC, htons(ETH_P_ALL));
    if (sock < 0) {
        fprintf(stderr, "socket() create raw socket failed: %s\n", strerror(errno));
        return -errno;
    }

    memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = if_nametoindex(iface);
    sll.sll_protocol = htons(ETH_P_ALL);
    if(bind(sock, (struct sockaddr *)&sll, sizeof(sll)) < 0) {
        fprintf(stderr, "bind() to '%s' failed: %s\n", iface, strerror(errno));
        close(sock);
        return -errno;
    }
    return sock;
}