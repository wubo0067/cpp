/*
 * @Author: CALM.WU
 * @Date: 2021-11-30 14:59:10
 * @Last Modified by:   CALM.WU
 * @Last Modified time: 2021-11-30 14:59:10
 */

#pragma once

#include <stdint.h>

#include "utils/clocks.h"

enum disk_type {
    DISK_TYPE_UNKNOWN,
    DISK_TYPE_PHYSICAL,
    DISK_TYPE_PARTITION,
    DISK_TYPE_VIRTUAL,
};

extern int32_t collector_proc_diskstats(int32_t update_every, usec_t dt);
