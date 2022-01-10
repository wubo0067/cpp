/*
 * @Author: CALM.WU
 * @Date: 2022-01-10 11:31:16
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2022-01-10 17:14:57
 */

#include "plugin_proc.h"

#include "utils/compiler.h"
#include "utils/consts.h"
#include "utils/log.h"
#include "utils/procfile.h"
#include "utils/strings.h"

#include "appconfig/appconfig.h"

static const char *__proc_stat_filename = "/proc/stat";

int32_t collector_proc_stat(int32_t update_every, usec_t dt, const char *config_path) {
    const char *f_stat =
        appconfig_get_member_str(config_path, "monitor_file", __proc_stat_filename);

    return 0;
}
