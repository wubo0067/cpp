/*
 * @Author: CALM.WU
 * @Date: 2021-10-15 10:20:43
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-10-15 17:44:04
 */

#pragma once

#include "common.h"

extern char pid_file[];

extern int32_t mk_daemon( int32_t dont_fork, const char* user );
extern int32_t become_user( const char* user, int32_t pid_fd );