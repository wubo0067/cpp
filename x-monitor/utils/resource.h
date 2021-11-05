/*
 * @Author: CALM.WU 
 * @Date: 2021-11-03 11:50:57 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-11-03 11:51:26
 */

#pragma once

#include <stdint.h>
#include <sys/types.h>

extern int32_t bump_memlock_rlimit(void);
extern const char *get_username(uid_t uid);