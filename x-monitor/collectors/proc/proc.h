/*
 * @Author: CALM.WU 
 * @Date: 2021-11-30 14:58:26 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-11-30 16:18:36
 */

#pragma once

#include "utils/common.h"

extern int32_t proc_routine_init();
extern void   *proc_routine_start(void *arg);
extern void    proc_routine_stop();

