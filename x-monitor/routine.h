/*
 * @Author: CALM.WU
 * @Date: 2021-10-22 14:49:37
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-10-22 16:32:23
 */

#pragma once

#include <pthread.h>

struct xmonitor_static_routine {
	const char* name;
	const char* config_name;
	pthread_t* thread;

	void ( *init_routine )();
	void* ( *start_routine )( void* );

	volatile sig_atomic_t exit_flag;

	struct xmonitor_static_routine* next;
};

extern void register_xmonitor_static_routine( struct xmonitor_static_routine* routine );