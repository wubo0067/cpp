/*
 * @Author: CALM.WU
 * @Date: 2022-01-10 10:49:20
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2022-01-13 17:08:07
 */

#include "plugin_proc.h"

#include "utils/compiler.h"
#include "utils/consts.h"
#include "utils/log.h"
#include "utils/procfile.h"
#include "utils/strings.h"

#include "appconfig/appconfig.h"

// http://brytonlee.github.io/blog/2014/05/07/linux-kernel-load-average-calc/

static const char *      __proc_loadavg_filename = "/proc/loadavg";
static struct proc_file *__pf_loadavg            = NULL;

int32_t collector_proc_loadavg(int32_t UNUSED(update_every), usec_t UNUSED(dt),
                               const char *config_path) {
    debug("[PLUGIN_PROC:proc_loadavg] config:%s running", config_path);

    const char *f_loadavg =
        appconfig_get_member_str(config_path, "monitor_file", __proc_loadavg_filename);

    if (unlikely(!__pf_loadavg)) {
        __pf_loadavg = procfile_open(f_loadavg, " \t,:|/", PROCFILE_FLAG_DEFAULT);
        if (unlikely(!__pf_loadavg)) {
            error("Cannot open %s", f_loadavg);
            return -1;
        }
    }

    __pf_loadavg = procfile_readall(__pf_loadavg);
    if (unlikely(!__pf_loadavg)) {
        error("Cannot read %s", f_loadavg);
        return -1;
    }

    if (unlikely(procfile_lines(__pf_loadavg) < 1)) {
        error("%s: File does not contain enough lines.", f_loadavg);
        return -1;
    }

    if (unlikely(procfile_linewords(__pf_loadavg, 0) < 6)) {
        error("%s: File does not contain enough columns.", f_loadavg);
        return -1;
    }

    double load_1m  = strtod(procfile_lineword(__pf_loadavg, 0, 0), NULL);
    double load_5m  = strtod(procfile_lineword(__pf_loadavg, 0, 1), NULL);
    double load_15m = strtod(procfile_lineword(__pf_loadavg, 0, 2), NULL);

    uint64_t running_processes = str2ull(procfile_lineword(__pf_loadavg, 0, 3));
    uint64_t total_processes   = str2ull(procfile_lineword(__pf_loadavg, 0, 4));
    pid_t    last_running_pid  = (pid_t)strtoll(procfile_lineword(__pf_loadavg, 0, 5), NULL, 10);

    debug("LOAD AVERAGE: %.2f %.2f %.2f running_processes: %lu total_processes: %lu "
          "last_running_pid: %d",
          load_1m, load_5m, load_15m, running_processes, total_processes, last_running_pid);

    return 0;
}