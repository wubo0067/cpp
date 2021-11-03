/*
 * @Author: CALM.WU 
 * @Date: 2021-11-03 11:37:51 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-11-03 16:42:01
 */

#include "ebpf_help.h"
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