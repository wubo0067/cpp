/*
 * @Author: CALM.WU 
 * @Date: 2021-11-25 16:23:49 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-11-29 11:42:41
 */
/*
 * @Author: CALM.WU 
 * @Date: 2021-11-25 16:23:44 
 * @Last Modified by:   CALM.WU 
 * @Last Modified time: 2021-11-25 16:23:44 
 */

#pragma once

#include <linux/version.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0))
#include "vmlinux-5.12.h"
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 18, 0))
#include "vmlinux-4.18.h"
#else 
#error "The kernel version is not supported"
#endif