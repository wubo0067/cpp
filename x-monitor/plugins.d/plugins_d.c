/*
 * @Author: CALM.WU
 * @Date: 2021-10-15 14:41:36
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-10-22 15:18:10
 */

#include "plugins.h"
#include "routine.h"
#include "utils/common.h"
#include "utils/compiler.h"
#include "utils/consts.h"
#include "utils/log.h"

#include "appconfig/appconfig.h"

struct external_plugin {
    char                    id[PLUGIN_ID_LEN + 1];
    char                    file_name[FILENAME_MAX + 1];
    volatile pid_t          pid;
    time_t                  start_time;
    volatile sig_atomic_t   enabled;
    int32_t                 exit_flag;
    pthread_t               thread;
    struct external_plugin *next;
};

struct pluginsd {
    struct external_plugin *external_plugins_root;
    const char             *plugins_dir;
    int32_t                 scan_frequency;
    int32_t                 exit_flag;
};

const int32_t DEFAULT_SCAN_MAX_FREQUENCY = 60;

static struct pluginsd __pluginsd = {
    .external_plugins_root = NULL,
    .scan_frequency        = DEFAULT_SCAN_MAX_FREQUENCY,
    .exit_flag             = 0,
};

static void external_plugin_cleanup(void *data)
{
    if (likely(__pluginsd.external_plugins_root)) {
        for (struct external_plugin *p = __pluginsd.external_plugins_root; p;
             p                         = p->next) {
            // 给线程发送信号，让线程退出，线程信号响应函数会执行killpid。
            pthread_cancel(p->thread);
        }
    }
    info("external_plugin_cleanup");
}

__attribute__((constructor)) static void register_route_pluginsd()
{
    struct xmonitor_static_routine *pr =
        (struct xmonitor_static_routine *)calloc(
            1, sizeof(struct xmonitor_static_routine));
    pr->name          = "PLUGINSD";
    pr->config_name   = NULL;
    pr->enabled       = 1;
    pr->init_routine  = pluginsd_routine_init;
    pr->start_routine = pluginsd_routine_start;
    pr->stop_routine  = pluginsd_routine_stop;
    register_xmonitor_static_routine(pr);
}

int32_t pluginsd_routine_init()
{
    debug("pluginsd_routine_init");
    // 插件目录
    __pluginsd.plugins_dir = appconfig_get_str("application.plugins_directory");
    if (unlikely(NULL == __pluginsd.plugins_dir)) {
        error("the application.plugins_directory is not configured");
        return -1;
    }
    debug("pluginsd_start plugins directory: %s", __pluginsd.plugins_dir);

    // 扫描频率
    __pluginsd.scan_frequency =
        appconfig_get_int("plugins_mgr.check_for_new_plugins_every");
    if (__pluginsd.scan_frequency <= 0) {
        __pluginsd.scan_frequency = 1;
    } else if (__pluginsd.scan_frequency > DEFAULT_SCAN_MAX_FREQUENCY) {
        __pluginsd.scan_frequency = DEFAULT_SCAN_MAX_FREQUENCY;
        warn(
            "the plugins_mgr.check_for_new_plugins_every: %d is too large, set to %d",
            __pluginsd.scan_frequency, DEFAULT_SCAN_MAX_FREQUENCY);
    }
    debug("pluginsd_start plugins scan_frequency: %d",
          __pluginsd.scan_frequency);

    return 0;
}

void *pluginsd_routine_start(void *arg)
{
    debug("pluginsd routine start");

    // https://www.cnblogs.com/guxuanqing/p/8385077.html
    // pthread_cleanup_push( pluginsd_cleanup, NULL );

    while (!__pluginsd.exit_flag) {
        sleep(__pluginsd.scan_frequency);
    }

    debug("pluginsd routine exit");
    return 0;
}

void pluginsd_routine_stop()
{
    __pluginsd.exit_flag = 1;
    pthread_join(__pluginsd.thread, NULL);

    if (likely(__pluginsd.plugins)) {
        for (struct external_plugin *p = __pluginsd.external_plugins_root; p;
             p                         = p->next) {
            p->exit_flag = 1;
            // 为了触发pthread_cleanup的注册函数，执行killpid
            pthread_cancel(p->thread);
            pthread_join(p->thread, NULL);
        }
    }
    debug("pluginsd routine stop");
}
