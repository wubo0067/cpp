/*
 * @Author: CALM.WU 
 * @Date: 2021-11-19 10:42:05 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-11-19 11:01:05
 */

#pragma once

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int32_t str_split_to_nums(const char *str, const char *delim,
                                 uint64_t *nums, uint16_t nums_max_size);

#ifdef __cplusplus
}
#endif