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
    // debug("[PLUGIN_PROC:proc_stat] /proc/stat line: %lu, core_index: %d", line, core_index);

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

    user_jiffies       = str2uint64_t(procfile_lineword(__pf_stat, line, 1));
    nice_jiffies       = str2uint64_t(procfile_lineword(__pf_stat, line, 2));
    system_jiffies     = str2uint64_t(procfile_lineword(__pf_stat, line, 3));
    idle_jiffies       = str2uint64_t(procfile_lineword(__pf_stat, line, 4));
    io_wait_jiffies    = str2uint64_t(procfile_lineword(__pf_stat, line, 5));
    irq_jiffies        = str2uint64_t(procfile_lineword(__pf_stat, line, 6));
    soft_irq_jiffies   = str2uint64_t(procfile_lineword(__pf_stat, line, 7));
    steal_jiffies      = str2uint64_t(procfile_lineword(__pf_stat, line, 8));
    guest_jiffies      = str2uint64_t(procfile_lineword(__pf_stat, line, 9));
    guest_nice_jiffies = str2uint64_t(procfile_lineword(__pf_stat, line, 10));

    user_jiffies -= guest_jiffies;
    nice_jiffies -= guest_nice_jiffies;

    debug("[PLUGIN_PROC:proc_stat] core_index: %d user_jiffies: %lu, nice_jiffies: %lu, "
          "system_jiffies: %lu, "
          "idle_jiffies: %lu, io_wait_jiffies: %lu, irq_jiffies: %lu, soft_irq_jiffies: %lu, "
          "steal_jiffies: %lu, guest_jiffies: %lu, guest_nice_jiffies: %lu",
          core_index, user_jiffies, nice_jiffies, system_jiffies, idle_jiffies, io_wait_jiffies,
          irq_jiffies, soft_irq_jiffies, steal_jiffies, guest_jiffies, guest_nice_jiffies);
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

    size_t lines      = procfile_lines(__pf_stat);
    size_t line_words = 0;

    uint64_t        interrupts_from_boot = 0, interrupts_per_sec = 0;
    static uint64_t interrupts_from_boot_last = 0;

    uint64_t        context_switches_from_boot = 0, context_switches_per_sec = 0;
    static uint64_t context_switches_from_boot_last = 0;

    debug("[PLUGIN_PROC:proc_stat] lines: %lu", lines);

    for (size_t index = 0; index < lines; index++) {
        char *row_name = procfile_lineword(__pf_stat, index, 0);

        if (likely(row_name[0] == 'c' && row_name[1] == 'p' && row_name[2] == 'u')) {
            line_words = procfile_linewords(__pf_stat, index);

            if (unlikely(line_words < 10)) {
                error("Cannot read /proc/stat cpu line. Expected 10 params, read %lu.", line_words);
                continue;
            }

            bool    is_core    = (row_name[3] != '\0');
            int32_t core_index = (is_core ? (row_name[3] - '0') : -1);
            do_cpu_utilization(index, core_index);

        } else if (unlikely(strncmp(row_name, "intr", 4) == 0)) {
            // “intr”这行给出中断的信息，第一个为自系统启动以来，发生的所有的中断的次数；然后每个数对应一个特定的中断自系统启动以来所发生的次数。
            // The first column is the total of all interrupts serviced; each subsequent column is
            // the total for that particular interrupt
            interrupts_from_boot = str2uint64_t(procfile_lineword(__pf_stat, index, 1));
            if (likely(0 != interrupts_from_boot_last
                       && interrupts_from_boot > interrupts_from_boot_last)) {
                // 如果中断计数器被重置了，那么我们将重置中断计数器的计数
                interrupts_per_sec = interrupts_from_boot - interrupts_from_boot_last;
            }
            interrupts_from_boot_last = interrupts_from_boot;
            debug("[PLUGIN_PROC:proc_stat] total_interrupts: %lu, interrupts/s: %lu",
                  interrupts_from_boot, interrupts_per_sec);

        } else if (unlikely(strncmp(row_name, "ctxt", 4) == 0)) {
            // The "ctxt" line gives the total number of context switches across all CPUs.
            context_switches_from_boot = str2uint64_t(procfile_lineword(__pf_stat, index, 1));
            if (likely(0 != context_switches_from_boot_last
                       && context_switches_from_boot > context_switches_from_boot_last)) {
                context_switches_per_sec =
                    context_switches_from_boot - context_switches_from_boot_last;
            }
            context_switches_from_boot_last = context_switches_from_boot;
            debug("[PLUGIN_PROC:proc_stat] total context switches: %lu, context switches/s: %lu",
                  context_switches_from_boot, context_switches_per_sec);

        } else if (unlikely(strncmp(row_name, "processes", 9) == 0)) {
            // The "processes" line gives the number of processes and threads created, which
            // includes (but is not limited to) those created by calls to the fork() and clone()
            // system calls.
            debug("[PLUGIN_PROC:proc_stat] processes");
        } else if (unlikely(strncmp(row_name, "procs_running", 13) == 0)) {
            // The "procs_running" line gives the number of processes currently running on CPUs.
            debug("[PLUGIN_PROC:proc_stat] processes running");
        } else if (unlikely(strncmp(row_name, "procs_blocked", 13) == 0)) {
            // The "procs_blocked" line gives the number of processes currently blocked, waiting for
            // I/O to complete.
            debug("[PLUGIN_PROC:proc_stat] processes blocked");
        }
    }

    return 0;
}
