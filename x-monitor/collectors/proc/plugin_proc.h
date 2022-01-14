/*
 * @Author: CALM.WU
 * @Date: 2021-11-30 14:58:26
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2022-01-13 16:20:44
 */

#pragma once

#include <stdint.h>

#include "utils/clocks.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int32_t proc_routine_init();
extern void *  proc_routine_start(void *arg);
extern void    proc_routine_stop();

enum disk_type {
    DISK_TYPE_UNKNOWN,
    DISK_TYPE_PHYSICAL,
    DISK_TYPE_PARTITION,
    DISK_TYPE_VIRTUAL,
};


extern int32_t collector_proc_diskstats(int32_t update_every, usec_t dt, const char *config_path);

extern int32_t collector_proc_stat(int32_t update_every, usec_t dt, const char *config_path);

extern int32_t collector_proc_loadavg(int32_t update_every, usec_t dt, const char *config_path);

#ifdef __cplusplus
}
#endif
