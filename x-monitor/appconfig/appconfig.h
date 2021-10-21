/*
 * @Author: CALM.WU
 * @Date: 2021-10-18 11:47:28
 * @Last Modified by:   CALM.WU
 * @Last Modified time: 2021-10-18 11:47:28
 */

#pragma once

#include "utils/common.h"

extern int32_t appconfig_load( const char* config_file );
extern void appconfig_destroy();

extern void appconfig_rdlock();
extern void appconfig_wrlock();
extern void appconfig_unlock();

// ---------------------------------------------------------
extern const char* appconfig_get_str( const char* key );