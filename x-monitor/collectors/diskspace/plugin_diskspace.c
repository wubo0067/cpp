/*
 * @Author: CALM.WU
 * @Date: 2021-12-20 11:16:02
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-12-20 17:40:34
 */

#include "plugin_proc.h"
#include "plugin_diskspace.h"

#include "utils/common.h"
#include "utils/compiler.h"
#include "utils/log.h"
#include "utils/clocks.h"
#include "utils/consts.h"
#include "routine.h"

#include "appconfig/appconfig.h"

static const char *__name = "PLUGIN_DISKSPACE";
static const char *__config_name = "collector_plugin_diskspace";

struct collector_diskspace {
    int32_t   exit_flag;
    pthread_t thread_id; // routine执行的线程id
};

static struct collector_diskspace __collector_diskspace = {
    .exit_flag = 0,
    .thread_id = 0,
};

__attribute__((constructor)) static void
collector_diskspace_register_routine() {
    fprintf(stderr, "---register_collector_diskspace_register_routine---\n");
    struct xmonitor_static_routine *xsr =
        (struct xmonitor_static_routine *)calloc(
            1, sizeof(struct xmonitor_static_routine));
    xsr->name = __name;
    xsr->config_name = __config_name; //配置文件中节点名
    xsr->enabled = 0;
    xsr->thread_id = &__collector_diskspace.thread_id;
    xsr->init_routine = diskspace_routine_init;
    xsr->start_routine = diskspace_routine_start;
    xsr->stop_routine = diskspace_routine_stop;
    register_xmonitor_static_routine(xsr);
}

int32_t diskspace_routine_init() {
    debug("[%s] routine init successed", __name);
    return 0;
}

void *diskspace_routine_start(void *arg) {
    debug("[%s] routine start", __name);
}

void diskspace_routine_stop() {
    __collector_diskspace.exit_flag = 1;
    pthread_join(__collector_diskspace.thread_id, NULL);
    debug("[%s] has completely stopped", __name);
    return;
}
