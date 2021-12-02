/*
 * @Author: CALM.WU 
 * @Date: 2021-11-30 14:59:18 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-11-30 17:16:44
 */

#include "proc.h"
#include "utils/compiler.h"
#include "utils/log.h"
#include "utils/clocks.h"
#include "utils/consts.h"
#include "routine.h"

#include "appconfig/appconfig.h"

static const char *__name        = "PLUGIN_PROC";
static const char *__config_name = "collector_proc";

struct proc_metrics_module {
    const char *name;
    int32_t     enabled;
    int32_t (*func)(int32_t update_every, usec_t dt);
};

struct collector_proc {
    int32_t                    exit_flag;
    pthread_t                  thread_id; // routine执行的线程id
    struct proc_metrics_module modules[];
};

static struct collector_proc
    __collector_proc = { .exit_flag = 0,
                         .thread_id = 0,
                         .modules   = {
                             // disk metrics
                             {
                                 .name    = "disk_metrics",
                                 .enabled = 1,
                                 .func    = NULL,
                             },
                             // the terminator of this array
                             { .name = NULL, .func = NULL },
                         } };

__attribute__((constructor)) static void collector_proc_register_routine()
{
    fprintf(stderr, "---register_collector_proc_routine---\n");
    struct xmonitor_static_routine *xsr =
        (struct xmonitor_static_routine *)calloc(
            1, sizeof(struct xmonitor_static_routine));
    xsr->name          = __name;
    xsr->config_name   = __config_name; //配置文件中节点名
    xsr->enabled       = 0;
    xsr->thread_id     = &__collector_proc.thread_id;
    xsr->init_routine  = proc_routine_init;
    xsr->start_routine = proc_routine_start;
    xsr->stop_routine  = proc_routine_stop;
    register_xmonitor_static_routine(xsr);
}

int32_t proc_routine_init()
{
    debug("[%s] routine init successed", __name);
    char proc_module_cfgname[CONFIG_NAME_MAX + 1];

    // check the enabled status for each module
    for (int32_t i = 0; __collector_proc.modules[i].name; i++) {
        //
        snprintf(proc_module_cfgname, CONFIG_NAME_MAX, "collector_proc.%s",
                 __collector_proc.modules[i].name);

        __collector_proc.modules[i].enabled =
            appconfig_get_member_bool(proc_module_cfgname, "enable", 1);
        debug("[%s] module %s is %s", __name, __collector_proc.modules[i].name,
              __collector_proc.modules[i].enabled ? "enabled" : "disabled");
    }
    return 0;
}

void *proc_routine_start(void *arg)
{
    debug("[%s] routine start", __name);

    while (!__collector_proc.exit_flag) {
        sleep(1);
    }

    debug("[%s] routine exit", __name);
    return NULL;
}

void proc_routine_stop()
{
    __collector_proc.exit_flag = 1;
    pthread_join(__collector_proc.thread_id, NULL);
    debug("[%s] has completely stopped", __name);
    return;
}