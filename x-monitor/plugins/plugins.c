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

struct __plugin_t {
	char id[PLUGIN_ID_LEN + 1];
	char file_name[FILENAME_MAX + 1];
	volatile pid_t pid;
	time_t start_time;
	volatile sig_atomic_t enabled;
	pthread_t thread;
	struct __plugin_t* next;
};

typedef struct {
	struct __plugin_t* plugins_root;
	int32_t curr_plugins_count;
	const char* plugins_dir;
	int32_t scan_frequency;
	int32_t exit_flag;
} __plugins_mgr_t;

const int32_t DEFAULT_SCAN_MAX_FREQUENCY = 60;

static __plugins_mgr_t __plugins_mgr = {
	.plugins_root       = NULL,
	.curr_plugins_count = 0,
	.scan_frequency     = DEFAULT_SCAN_MAX_FREQUENCY,
	.exit_flag          = 0,
};

static void pluginsmgr_cleanup( void* data ) {
	if ( likely( __plugins_mgr.plugins ) ) {
		for ( struct __plugin_t* p = __plugins_mgr.plugins_root; p; p = p->next ) {
			pthread_cancel( p->thread );
		}
	}
	info( "pluginsmgr_cleanup" );
}

__attribute__( ( constructor ) ) static void register_route_pluginsmgr() {
	struct xmonitor_static_routine* pr
	    = ( struct xmonitor_static_routine* ) calloc( 1, sizeof( struct xmonitor_static_routine ) );
	pr->name          = "PLUGINSMGR";
	pr->init_routine  = pluginsmgr_routine_init;
	pr->start_routine = pluginsmgr_routine_start;
	pr->stop_routine  = pluginsmgr_routine_stop;
	register_xmonitor_static_routine( pr );
}

bool pluginsmgr_routine_init() {
	debug( "pluginsmgr_routine_init" );
	// 插件目录
	__plugins_mgr.plugins_dir = appconfig_get_str( "application.plugins_directory" );
	if ( unlikely( NULL == __plugins_mgr.plugins_dir ) ) {
		error( "the application.plugins_directory is not configured" );
		return false;
	}
	debug( "pluginsd_start plugins directory: %s", __plugins_mgr.plugins_dir );

	// 扫描频率
	__plugins_mgr.scan_frequency = appconfig_get_int( "plugins_mgr.check_for_new_plugins_every" );
	if ( __plugins_mgr.scan_frequency <= 0 ) {
		__plugins_mgr.scan_frequency = 1;
	}
	else if ( __plugins_mgr.scan_frequency > DEFAULT_SCAN_MAX_FREQUENCY ) {
		__plugins_mgr.scan_frequency = DEFAULT_SCAN_MAX_FREQUENCY;
		warn( "the plugins_mgr.check_for_new_plugins_every: %d is too large, set to %d", __plugins_mgr.scan_frequency,
		    DEFAULT_SCAN_MAX_FREQUENCY );
	}
	debug( "pluginsd_start plugins scan_frequency: %d", __plugins_mgr.scan_frequency );

	return true;
}

void* pluginsmgr_routine_start( void* arg ) {
	debug( "pluginsmgr_routine_start" );

	// https://www.cnblogs.com/guxuanqing/p/8385077.html
	pthread_cleanup_push( pluginsmgr_cleanup, NULL );

	while ( !__plugins_mgr.exit_flag ) {
		sleep( __plugins_mgr.scan_frequency );
	}

	// 执行push的清理函数
	pthread_cleanup_pop( 1 );
	debug( "pluginsmgr_routine_start exit" );
	return 0;
}

void pluginsmgr_routine_stop() {
	debug( "pluginsmgr_routine_stop" );
	__plugins_mgr.exit_flag = 1;
}
