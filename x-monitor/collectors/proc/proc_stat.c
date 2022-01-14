/*
 * @Author: CALM.WU
 * @Date: 2022-01-10 11:31:16
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2022-01-13 17:24:03
 */

// https://www.jianshu.com/p/aea52895de5e

#include "plugin_proc.h"

#include "utils/compiler.h"
#include "utils/consts.h"
#include "utils/log.h"
#include "utils/procfile.h"
#include "utils/strings.h"

#include "appconfig/appconfig.h"

int32_t processors = 1;

static const char *      __proc_stat_filename = "/proc/stat";
static struct proc_file *__pf_stat            = NULL;

static bool __do_cpu_utilization, __do_per_cpu_core_utilization, __do_context_switches,
    __do_cpu_interrupts, __do_processes_fork, __do_processes_running;

static void __set_do_config(const char *config_path) {
    __do_cpu_utilization = appconfig_get_member_bool(config_path, "cpu_utilization", true);
    __do_per_cpu_core_utilization =
        appconfig_get_member_bool(config_path, "per_cpu_core_utilization", true);
    __do_context_switches  = appconfig_get_member_bool(config_path, "context_switches", true);
    __do_cpu_interrupts    = appconfig_get_member_bool(config_path, "cpu_interrupts", true);
    __do_processes_fork    = appconfig_get_member_bool(config_path, "processes_fork", true);
    __do_processes_running = appconfig_get_member_bool(config_path, "processes_running", true);
}

static void do_cpu_utilization(size_t line, int32_t core_index) {
    debug("[PLUGIN_PROC:proc_stat] /proc/stat line: %lu, core_index: %d", line, core_index);

    // sysconf(_SC_CLK_TCK)一般地定义为jiffies(一般地等于10ms)
    // CPU时间 = user + system + nice + idle + iowait + irq + softirq

    uint64_t user_jiffies,  // 用户态时间
        nice_jiffies,       // nice用户态时间
        system_jiffies,     // 系统态时间
        idle_jiffies,       // 空闲时间, 不包含IO等待时间
        io_wait_jiffies,    // IO等待时间
        irq_jiffies,        // 硬中断时间
        soft_irq_jiffies,   // 软中断时间
        steal_jiffies,  // 虚拟化环境中运行其他操作系统上花费的时间（since Linux 2.6.11）
        guest_jiffies,       // 操作系统运行虚拟CPU花费的时间（since Linux 2.6.24）
        guest_nice_jiffies;  // 运行一个带nice值的guest花费的时间（since Linux 2.6.24）
}

int32_t collector_proc_stat(int32_t update_every, usec_t dt, const char *config_path) {
    debug("[PLUGIN_PROC:proc_stat] config:%s running", config_path);
    // 设置配置
    __set_do_config(config_path);

    const char *f_stat =
        appconfig_get_member_str(config_path, "monitor_file", __proc_stat_filename);

    if (unlikely(!__pf_stat)) {
        __pf_stat = procfile_open(f_stat, " \t:", PROCFILE_FLAG_DEFAULT);
        if (unlikely(!__pf_stat)) {
            error("Cannot open %s", f_stat);
            return -1;
        }
    }

    __pf_stat = procfile_readall(__pf_stat);
    if (unlikely(!__pf_stat)) {
        error("Cannot read %s", f_stat);
        return -1;
    }

    size_t lines = procfile_lines(__pf_stat);
    size_t words = 0;

    debug("[PLUGIN_PROC:proc_stat] lines: %lu", lines);

    for (size_t index = 0; index < lines; index++) {
        char *row_name = procfile_lineword(__pf_stat, index, 0);

        if (likely(row_name[0] == 'c' && row_name[1] == 'p' && row_name[2] == 'u')) {
            words = procfile_linewords(__pf_stat, index);

            bool    is_core    = (row_name[3] != '\0');
            int32_t core_index = (is_core ? (row_name[3] - '0') : -1);
            do_cpu_utilization(index, core_index);

        } else if (unlikely(strncmp(row_name, "intr", 4) == 0)) {
            debug("[PLUGIN_PROC:proc_stat] interrupt");
        } else if (unlikely(strncmp(row_name, "ctxt", 4) == 0)) {
            debug("[PLUGIN_PROC:proc_stat] context switches");
        } else if (unlikely(strncmp(row_name, "btime", 5) == 0)) {
            debug("[PLUGIN_PROC:proc_stat] boot time");
        } else if (unlikely(strncmp(row_name, "processes", 9) == 0)) {
            // 自系统启动以来所创建的任务的个数目
            debug("[PLUGIN_PROC:proc_stat] processes");
        } else if (unlikely(strncmp(row_name, "procs_running", 13) == 0)) {
            // 当前运行队列的任务的数目
            debug("[PLUGIN_PROC:proc_stat] processes running");
        } else if (unlikely(strncmp(row_name, "procs_blocked", 13) == 0)) {
            // 当前被阻塞的任务的数目
            debug("[PLUGIN_PROC:proc_stat] processes blocked");
        }
    }

    return 0;
}
