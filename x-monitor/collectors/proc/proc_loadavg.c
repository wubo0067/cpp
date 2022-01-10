/*
 * @Author: CALM.WU
 * @Date: 2022-01-10 10:49:20
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2022-01-10 17:07:45
 */

#include "plugin_proc.h"

#include "utils/compiler.h"
#include "utils/consts.h"
#include "utils/log.h"
#include "utils/procfile.h"
#include "utils/strings.h"

#include "appconfig/appconfig.h"

// http://brytonlee.github.io/blog/2014/05/07/linux-kernel-load-average-calc/

static const char *__proc_loadavg_filename = "/proc/loadavg";

int32_t collector_proc_loadavg(int32_t UNUSED(update_every), usec_t dt, const char *config_path) {
    const char *f_loadavg =
        appconfig_get_member_str(config_path, "monitor_file", __proc_loadavg_filename);

    return 0;
}