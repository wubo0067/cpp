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

typedef struct {
	char file_name[FILENAME_MAX + 1];
	volatile pid_t pid;
	time_t start_time;
	volatile sig_atomic_t enabled;
} __plugin_t;

typedef struct {
	__plugin_t* plugins;
	const int32_t max_plugins_count;
	int32_t curr_plugins_count;
} __plugins_root_t;

static __plugins_root_t __plugins_root = {
	.plugins            = NULL,
	.max_plugins_count  = 0,
	.curr_plugins_count = 0,
	.exit_flag          = 0,
};

static void pluginsmgr_cleanup( void* data ) {
	if ( likely( __plugins_root.plugins ) ) {
		free( __plugins_root.plugins );
		__plugins_root.plugins = NULL;
	}
	info( "pluginsmgr_cleanup" );
}

void* pluginsmgr_main( void* arg ) {

	xmonitor_static_routine_t* pluginsmgr_routine = ( xmonitor_static_routine_t* ) arg;
	int32_t exit_flag                             = pluginsmgr_routine->exit_flag;

	const char* plugins_dir = appconfig_get_str( "application.plugins_directory" );
	if ( unlikely( NULL == plugins_dir ) ) {
		error( "the application.plugins_directory is not configured" );
		return NULL;
	}
	debug( "pluginsd_start plugins directory: %s", plugings_dir );

	int32_t plugins_count_max = appconfig_get_int( "plugins_mgr.max_count" );
	if ( unlikely( plugins_count_max <= 0 ) ) {
		error( "the plugins_mgr.max_count: %d invalid configuration", plugins_count_max );
		return NULL;
	}

	int32_t scan_frequency = appconfig_get_int( "plugins_mgr.check_for_new_plugins_every" );
	if ( scan_frequency <= 0 ) {
		scan_frequency = 1;
	}

	__plugins_root.plugins = calloc( plugins_count_max, sizeof( __plugin_t ) );

	pthread_cleanup_push( pluginsmgr_cleanup, NULL );

	while ( !exit_flag ) {
		sleep( scan_frequency );
	}

	pthread_cleanup_pop( 1 );
	return 0;
}
