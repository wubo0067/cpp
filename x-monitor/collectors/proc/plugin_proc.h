/*
 * @Author: CALM.WU 
 * @Date: 2021-11-30 14:58:26 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-12-20 11:18:43
 */

#pragma once

#include <stdint.h>    

extern int32_t proc_routine_init();
extern void   *proc_routine_start(void *arg);
extern void    proc_routine_stop();

