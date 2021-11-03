/*
 * @Author: CALM.WU 
 * @Date: 2021-11-03 11:51:34 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-11-03 11:51:54
 */

#include "resource.h"
#include "common.h"

int32_t bump_memlock_rlimit(void)
{
    struct rlimit rlim_new = {
        .rlim_cur = RLIM_INFINITY,
        .rlim_max = RLIM_INFINITY,
    };

    return setrlimit(RLIMIT_MEMLOCK, &rlim_new);
}