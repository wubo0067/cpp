/*
 * @Author: CALM.WU 
 * @Date: 2021-10-15 10:26:46 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-10-21 11:26:48
 */

#pragma once

#include <stdint.h>

typedef void (*clean_and_exit_handle_fn)(int32_t signo);

extern void    signals_init(void);
extern void    signals_handle(clean_and_exit_handle_fn fn);
extern int32_t signals_block();
extern int32_t signals_unblock();