/*
 * @Author: CALM.WU 
 * @Date: 2021-10-15 10:20:43 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-10-15 13:55:49
 */

#pragma once

#include "common.h"

extern char pid_file[];

extern int32_t daemon(int32_t dont_fork, const char *user);