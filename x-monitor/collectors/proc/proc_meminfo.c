/*
 * @Author: CALM.WU
 * @Date: 2022-01-26 17:00:40
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2022-01-26 17:01:11
 */

// 取得物理内存信息（相关文件/proc/meminfo）

#include "plugin_proc.h"

#include "prometheus-client-c/prom.h"

#include "utils/compiler.h"
#include "utils/consts.h"
#include "utils/log.h"
#include "utils/procfile.h"
#include "utils/strings.h"

#include "appconfig/appconfig.h"

int32_t init_collector_proc_meminfo() {
    return 0;
}

int32_t collector_proc_meminfo(int32_t update_every, usec_t dt, const char *config_path) {
    return 0;
}

void fini_collector_proc_meminfo() {
}