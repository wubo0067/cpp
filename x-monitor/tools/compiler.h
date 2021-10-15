/*
 * @Author: CALM.WU 
 * @Date: 2021-10-14 16:33:29 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-10-15 17:46:34
 */

#pragma once

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)