/*
 * @Author: CALM.WU
 * @Date: 2021-10-15 14:41:36
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-10-22 15:18:10
 */

#include "plugins_d.h"
#include "routine.h"
#include "utils/common.h"
#include "utils/compiler.h"
#include "utils/consts.h"
#include "utils/log.h"
#include "utils/clocks.h"

#include "appconfig/appconfig.h"

#define PLUGINSD_FILE_SUFFIX ".plugin"
#define PLUGINSD_FILE_SUFFIX_LEN strlen(PLUGINSD_FILE_SUFFIX)

struct external_plugin {
    char id[CONFIG_NAME_MAX + 1];
    char file_name[FILENAME_MAX + 1];
    char full_file_name[FILENAME_MAX + 1];
    char cmd[EXTERNAL_PLUGIN_CMD_LINE_MAX + 1]; // the command that it executes

    volatile pid_t        pid; //
    time_t                start_time;
    volatile sig_atomic_t enabled;
    int32_t               exit_flag;
    pthread_t             thread_id;

    struct external_plugin *next;
};

struct pluginsd {
    struct external_plugin *external_plugins_root;
    const char             *plugins_dir;
    int32_t                 scan_frequency;
    int32_t                 exit_flag;
    pthread_t               thread_id;
};

const int32_t DEFAULT_SCAN_MAX_FREQUENCY = 60;

static struct pluginsd __pluginsd = {
    .external_plugins_root = NULL,
    .scan_frequency        = DEFAULT_SCAN_MAX_FREQUENCY,
    .exit_flag             = 0,
};

__attribute__((constructor)) static void pluginsd_register_routine()
{
    fprintf(stderr, "---register_pluginsd_routine---\n");
    struct xmonitor_static_routine *pr =
        (struct xmonitor_static_routine *)calloc(
            1, sizeof(struct xmonitor_static_routine));
    pr->name          = "PLUGINSD";
    pr->config_name   = NULL;
    pr->enabled       = 1;
    pr->thread_id     = &__pluginsd.thread_id;
    pr->init_routine  = pluginsd_routine_init;
    pr->start_routine = pluginsd_routine_start;
    pr->stop_routine  = pluginsd_routine_stop;
    register_xmonitor_static_routine(pr);
}

static void external_plugin_cleanup(void *data)
{
    for (struct external_plugin *p = __pluginsd.external_plugins_root; p;
         p                         = p->next) {
        // 给线程发送信号，让线程退出，线程信号响应函数会执行killpid。
        pthread_cancel(p->thread_id);
    }
    info("external_plugin_cleanup");
}

int32_t pluginsd_routine_init()
{
    debug("pluginsd_routine_init");
    // 插件目录
    __pluginsd.plugins_dir =
        appconfig_get_str("application.plugins_directory", NULL);
    if (unlikely(NULL == __pluginsd.plugins_dir)) {
        error("the application.plugins_directory is not configured");
        return -1;
    }
    debug("application.plugins_directory: %s", __pluginsd.plugins_dir);

    // 扫描频率
    __pluginsd.scan_frequency =
        appconfig_get_int("pluginsd.check_for_new_plugins_every", 5);
    if (__pluginsd.scan_frequency <= 0) {
        __pluginsd.scan_frequency = 1;
    } else if (__pluginsd.scan_frequency > DEFAULT_SCAN_MAX_FREQUENCY) {
        __pluginsd.scan_frequency = DEFAULT_SCAN_MAX_FREQUENCY;
        warn(
            "the pluginsd.check_for_new_plugins_every: %d is too large, set to %d",
            __pluginsd.scan_frequency, DEFAULT_SCAN_MAX_FREQUENCY);
    }
    debug("pluginsd.check_for_new_plugins_every: %d",
          __pluginsd.scan_frequency);

    return 0;
}

void *pluginsd_routine_start(void *arg)
{
    debug("pluginsd routine start");

    // https://www.cnblogs.com/guxuanqing/p/8385077.html
    // pthread_cleanup_push( pluginsd_cleanup, NULL );

    while (!__pluginsd.exit_flag) {
        // 扫描插件目录，找到执行程序
        DIR *plugins_dir = opendir(__pluginsd.plugins_dir);
        if (unlikely(NULL == plugins_dir)) {
            error("cannot open plugins directory '%s'", __pluginsd.plugins_dir);
            continue;
        }

        struct dirent *entry = NULL;
        while (likely(NULL != (entry = readdir(plugins_dir)))) {
            if (unlikely(__pluginsd.exit_flag)) {
                break;
            }

            debug("pluginsd check examining file '%s'", entry->d_name);

            if (unlikely(strcmp(entry->d_name, ".") == 0 ||
                         strcmp(entry->d_name, "..") == 0))
                continue;

            // 检查文件名是否以.plugin结尾
            size_t len = strlen(entry->d_name);
            if (unlikely(len <= PLUGINSD_FILE_SUFFIX_LEN))
                continue;

            if (unlikely(strcmp(entry->d_name + len - PLUGINSD_FILE_SUFFIX_LEN,
                                PLUGINSD_FILE_SUFFIX) != 0)) {
                debug("file '%s' does not end in '%s'", entry->d_name,
                      PLUGINSD_FILE_SUFFIX);
                continue;
            }

            // 检查配置是否可以运行
            char external_plugin_cfgname[CONFIG_NAME_MAX + 1];
            snprintf(external_plugin_cfgname, CONFIG_NAME_MAX, "pluginsd.%.*s",
                     (int)(len - PLUGINSD_FILE_SUFFIX_LEN), entry->d_name);
            int32_t enabled = appconfig_get_int(external_plugin_cfgname, 0);

            if (unlikely(!enabled)) {
                debug("external plugin config '%s' is not enabled",
                      external_plugin_cfgname);
                continue;
            }

            // 判断是否已经在运行
            struct external_plugin *ep = NULL;
            for (ep = __pluginsd.external_plugins_root; ep; ep = ep->next) {
                if (unlikely(0 == strcmp(ep->file_name, entry->d_name))) {
                    break;
                }
            }
            if (ep) {
                debug("external plugin '%s' is already running", entry->d_name);
                continue;
            }

            // 没有运行，启动
            if (!ep) {
                ep = (struct external_plugin *)calloc(
                    1, sizeof(struct external_plugin));

                snprintf(ep->id, CONFIG_NAME_MAX, "plugin:%s",
                         external_plugin_cfgname);
                strncpy(ep->file_name, entry->d_name, FILENAME_MAX);
                snprintf(ep->full_file_name, FILENAME_MAX, "%s/%s",
                         __pluginsd.plugins_dir, entry->d_name);
                // 检查文件是否可执行
                if (unlikely(access(ep->full_file_name, X_OK) != 0)) {
                    warn("cannot execute file '%s'", ep->full_file_name);
                    free(ep);
                    ep = NULL;
                    continue;
                }

                ep->enabled    = enabled;
                ep->start_time = now_realtime_sec();
                // 创建运行线程
            }
        }
        sleep(__pluginsd.scan_frequency);
    }

    debug("pluginsd routine exit");
    return 0;
}

void pluginsd_routine_stop()
{
    __pluginsd.exit_flag = 1;
    pthread_join(__pluginsd.thread_id, NULL);

    for (struct external_plugin *p = __pluginsd.external_plugins_root; p;
         p                         = p->next) {
        p->exit_flag = 1;
        // 为了触发pthread_cleanup的注册函数，执行killpid
        pthread_cancel(p->thread_id);
        pthread_join(p->thread_id, NULL);
    }
    debug("routine 'Pluginsd' has completely stopped");
}
