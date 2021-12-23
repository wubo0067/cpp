/*
 * @Author: CALM.WU
 * @Date: 2021-12-20 11:16:02
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-12-20 17:40:34
 */

#include "plugin_diskspace.h"
#include "plugin_proc.h"

#include "routine.h"
#include "utils/clocks.h"
#include "utils/common.h"
#include "utils/compiler.h"
#include "utils/consts.h"
#include "utils/log.h"
#include "utils/mountinfo.h"

#include "appconfig/appconfig.h"

static const char *__name        = "PLUGIN_DISKSPACE";
static const char *__config_name = "collector_plugin_diskspace";

struct collector_diskspace {
    int32_t           exit_flag;
    pthread_t         thread_id;  // routine执行的线程ids
    int32_t           update_every;
    int32_t           check_for_new_mountinfos_every;
    struct mountinfo *disk_mountinfo_root;
};

static struct collector_diskspace __collector_diskspace = {
    .exit_flag                      = 0,
    .thread_id                      = 0,
    .update_every                   = 1,
    .check_for_new_mountinfos_every = 15,
    .disk_mountinfo_root            = NULL,
};

__attribute__((constructor)) static void collector_diskspace_register_routine() {
    fprintf(stderr, "---register_collector_diskspace_register_routine---\n");
    struct xmonitor_static_routine *xsr =
        ( struct xmonitor_static_routine * )calloc(1, sizeof(struct xmonitor_static_routine));
    xsr->name          = __name;
    xsr->config_name   = __config_name;  //配置文件中节点名
    xsr->enabled       = 0;
    xsr->thread_id     = &__collector_diskspace.thread_id;
    xsr->init_routine  = diskspace_routine_init;
    xsr->start_routine = diskspace_routine_start;
    xsr->stop_routine  = diskspace_routine_stop;
    register_xmonitor_static_routine(xsr);
}

static void __reload_mountinfo(int32_t force) {
    static time_t last_load = 0;
    time_t        now       = now_realtime_sec();

    if (force || now - last_load >= __collector_diskspace.check_for_new_mountinfos_every) {
        // 先释放
        mountinfo_free_all(__collector_diskspace.disk_mountinfo_root);
        __collector_diskspace.disk_mountinfo_root = mountinfo_read(0);
        last_load                                 = now;
    }
}

static void __collector_diskspace_stats(struct mountinfo *mi, int32_t update_every) {}

int32_t diskspace_routine_init() {
    debug("[%s] routine init successed", __name);

    __collector_diskspace.update_every = appconfig_get_int("collector_plugin_diskspace.update_every", 1);
    __collector_diskspace.check_for_new_mountinfos_every =
        appconfig_get_int32(__config_name, "check_for_new_mountinfos_every", 15);

    debug("[%s] routine start, update_every: %d, check_for_new_mountpoints_every: %d", __name,
          __collector_diskspace.update_every, __collector_diskspace.check_for_new_mountpoints_every);
    return 0;
}

void *diskspace_routine_start(void *arg) {
    debug("[%s] routine start", __name);

    uset_t duration          = 0;
    uset_t step_microseconds = __collector_diskspace.update_every * USEC_PER_SEC;

    struct heartbeat hb;
    heartbeat_init(&hb);

    while (!__collector_diskspace.exit_flag) {
        //等到下一个update周期
        heartbeat_next(&hb, step_microseconds);

        if (__collector_diskspace.exit_flag) {
            break;
        }

        // 读取/proc/self/mountinfo，获取挂载文件系统信息
        __reload_mountinfo(0);

        //--------------------------------------------------------------------------------
        // disk space metrics
        struct mountinfo *mi;
        for (mi = __collector_diskspace.disk_mountinfo_root; mi; mi->next) {
            if (unlikely(mi->flags & (MOUNTINFO_FLAG_IS_DUMMY | MOUNTINFO_FLAG_IS_BIND))) {
                // 忽略指定的文件系统和绑定文件系统
                continue;
            }

            __collector_diskspace_stats(mi, __collector_diskspace.update_every);

            if (unlikely(__collector_diskspace.exit_flag))
                break;
        }
    }
}

void diskspace_routine_stop() {
    __collector_diskspace.exit_flag = 1;
    pthread_join(__collector_diskspace.thread_id, NULL);
    debug("[%s] has completely stopped", __name);
    return;
}
