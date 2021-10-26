/*
 * @Author: CALM.WU
 * @Date: 2021-10-18 11:47:42
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-10-22 14:35:15
 */

#include "appconfig.h"
#include "utils/common.h"
#include "utils/compiler.h"
#include "utils/log.h"

#include "libconfig/libconfig.h"

typedef struct {
    config_t         cfg;
    bool             loaded;
    pthread_rwlock_t rw_lock;
} __appconfig_t;

static __appconfig_t  __appconfig;
static pthread_once_t __appconfig_is_initialized = PTHREAD_ONCE_INIT;

static void __appconfig_init(void)
{
    pthread_rwlock_init(&__appconfig.rw_lock, NULL);
    config_init(&__appconfig.cfg);
    __appconfig.loaded = false;
}

int32_t appconfig_load(const char *config_file)
{
    int32_t ret = 0;

    info("appconfig_load: %s", config_file);
    // once initialize
    pthread_once(&__appconfig_is_initialized, __appconfig_init);

    pthread_rwlock_wrlock(&__appconfig.rw_lock);

    // 加载配置数据
    if (config_read_file(&__appconfig.cfg, config_file) != CONFIG_TRUE) {
        fprintf(stderr, "config_read_file failed: %s:%d - %s",
                config_error_file(&__appconfig.cfg),
                config_error_line(&__appconfig.cfg),
                config_error_text(&__appconfig.cfg));
        __appconfig.loaded = false;
        config_destroy(&__appconfig.cfg);
        ret = -1;
    } else {
        __appconfig.loaded = true;
    }
    pthread_rwlock_unlock(&__appconfig.rw_lock);

    if (likely(0 == ret)) {
        // 输出配置文件
        config_write(&__appconfig.cfg, stdout);
    }

    return ret;
}

void appconfig_destroy()
{
    pthread_rwlock_wrlock(&__appconfig.rw_lock);
    config_destroy(&__appconfig.cfg);
    pthread_rwlock_unlock(&__appconfig.rw_lock);
    pthread_rwlock_destroy(&__appconfig.rw_lock);
}

// ----------------------------------------------------------------------------
// locking

// void appconfig_rdlock() { pthread_rwlock_rdlock( &__appconfig.rw_lock ); }

// void appconfig_wrlock() { pthread_rwlock_wrlock( &__appconfig.rw_lock ); }

// void appconfig_unlock() { pthread_rwlock_unlock( &__appconfig.rw_lock ); }

// ----------------------------------------------------------------------------

const char *appconfig_get_str(const char *key)
{
    if (unlikely(!key)) {
        return NULL;
    }

    const char *str = NULL;
    pthread_rwlock_rdlock(&__appconfig.rw_lock);
    if (likely(__appconfig.loaded)) {
        if (!config_lookup_string(&__appconfig.cfg, key, &str)) {
            error("config_lookup_string failed: %s", key);
        }
    }
    pthread_rwlock_unlock(&__appconfig.rw_lock);
    return str;
}

int32_t appconfig_get_bool(const char *key)
{
    if (unlikely(!key)) {
        return 0;
    }

    int32_t b;
    pthread_rwlock_rdlock(&__appconfig.rw_lock);
    if (likely(__appconfig.loaded)) {
        if (!config_lookup_bool(&__appconfig.cfg, key, &b)) {
            error("config_lookup_bool failed: %s", key);
        }
    }
    pthread_rwlock_unlock(&__appconfig.rw_lock);
    return b;
}

int32_t appconfig_get_int(const char *key)
{
    if (unlikely(!key)) {
        return 0;
    }

    int32_t i;
    pthread_rwlock_rdlock(&__appconfig.rw_lock);
    if (likely(__appconfig.loaded)) {
        if (!config_lookup_int(&__appconfig.cfg, key, &i)) {
            error("config_lookup_int failed: %s", key);
        }
    }
    pthread_rwlock_unlock(&__appconfig.rw_lock);
    return i;
}