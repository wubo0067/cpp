/*
 * @Author: CALM.WU
 * @Date: 2021-10-12 10:44:47
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-10-21 14:59:14
 */

#include "config.h"

#include "utils/common.h"
#include "utils/compiler.h"
#include "utils/daemon.h"
#include "utils/log.h"
#include "utils/popen.h"
#include "utils/signals.h"

#include "plugins.d/plugins_d.h"

#include "appconfig/appconfig.h"

#define BUF_SIZE 1024

struct option_def {
	/** The option character */
	const char val;
	/** The name of the long option. */
	const char* description;
	/** Short description what the option does */
	/** Name of the argument displayed in SYNOPSIS */
	const char* arg_name;
	/** Default value if not set */
	const char* default_value;
};

static const struct option_def option_definitions[] = {
	// opt description     arg name       default value
	{ 'c', "Configuration file to load.", "filename", CONFIG_FILENAME },
	{ 'D', "Do not fork. Run in the foreground.", NULL, "run in the background" },
	{ 'h', "Display this help message.", NULL, NULL },
	{ 'i', "The IP address to listen to.", "IP", "all IP addresses IPv4 and IPv6" },
	{ 'p', "API/Web port to use.", "port", "19999" },
	{ 's', "Prefix for /proc and /sys (for containers).", "path", "no prefix" },
	{ 't', "The internal clock of netdata.", "seconds", "1" }, { 'u', "Run as user.", "username", "netdata" },
	{ 'v', "Print netdata version and exit.", NULL, NULL }, { 'V', "Print netdata version and exit.", NULL, NULL }
};

static const char * pid_file = NULL;

// killpid kills pid with SIGTERM.
int killpid( pid_t pid ) {
	int ret;
	debug( "Request to kill pid %d", pid );
	debug( "-----------------" );

	errno = 0;
	ret   = kill( pid, SIGTERM );
	if ( ret == -1 ) {
		switch ( errno ) {
			case ESRCH:
				// We wanted the process to exit so just let the caller handle.
				return ret;
			case EPERM:
				error( "Cannot kill pid %d, but I do not have enough permissions.", pid );
				break;
			default:
				error( "Cannot kill pid %d, but I received an error.", pid );
				break;
		}
	}
	return ret;
}

void help() {
	int32_t num_opts = ( int32_t ) ARRAY_SIZE( option_definitions );
	int32_t i;
	int32_t max_len_arg = 0;

	// Compute maximum argument length
	for ( i = 0; i < num_opts; i++ ) {
		if ( option_definitions[i].arg_name ) {
			int len_arg = ( int ) strlen( option_definitions[i].arg_name );
			if ( len_arg > max_len_arg )
				max_len_arg = len_arg;
		}
	}

	if ( max_len_arg > 30 )
		max_len_arg = 30;
	if ( max_len_arg < 20 )
		max_len_arg = 20;

	fprintf( stderr, "%s",
	    "\n"
	    " ^\n"
	    " |.-.   .-.   .-.   .-.   .  x-monitor                                         \n"
	    " |   '-'   '-'   '-'   '-'   real-time performance monitoring, done right!   \n"
	    " +----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+--->\n"
	    "\n"
	    " Copyright (C) 2021-2100, Calm.wu\n"
	    " Released under GNU General Public License v3 or later.\n"
	    " All rights reserved.\n" );

	fprintf( stderr, " SYNOPSIS: x-monitor [options]\n" );
	fprintf( stderr, "\n" );
	fprintf( stderr, " Options:\n\n" );

	// Output options description.
	for ( i = 0; i < num_opts; i++ ) {
		fprintf( stderr, "  -%c %-*s  %s", option_definitions[i].val, max_len_arg,
		    option_definitions[i].arg_name ? option_definitions[i].arg_name : "", option_definitions[i].description );
		if ( option_definitions[i].default_value ) {
			fprintf( stderr, "\n   %c %-*s  Default: %s\n", ' ', max_len_arg, "", option_definitions[i].default_value );
		}
		else {
			fprintf( stderr, "\n" );
		}
		// fprintf( stderr, "\n" );
	}

	fflush( stderr );
	return;
}

static void main_cleanup_and_exit( int32_t UNUSED( signo ) ) {

	if ( pid_file != NULL && pid_file[0] != '\0' ) {
		info( "EXIT: removing pid file '%s'", pid_file );
		if ( unlink( pid_file ) != 0 )
			error( "EXIT: cannot remove pid file '%s'", pid_file );
	}

	// free config
	appconfig_destroy();
	// free log
	log_fini();
}

int32_t main( int32_t argc, char* argv[] ) {
	char UNUSED( buf[BUF_SIZE] ) = { 0 };
	pid_t UNUSED( child_pid )    = 0;
	int32_t dont_fork            = 0;
	int32_t config_loaded        = 0;

	// parse options
	{
		int32_t opts_count = ( int32_t ) ARRAY_SIZE( option_definitions );
		char opt_str[( opts_count * 2 ) + 1];

		int32_t opt_str_i = 0;
		for ( int32_t i = 0; i < opts_count; i++ ) {
			opt_str[opt_str_i] = option_definitions[i].val;
			opt_str_i++;
			if ( option_definitions[i].arg_name ) {
				opt_str[opt_str_i++] = ':';
				opt_str_i++;
			}
		}

		// terminate optstring
		opt_str[opt_str_i]          = '\0';
		opt_str[( opts_count * 2 )] = '\0';

		int32_t opt = 0;

		while ( ( opt = getopt( argc, argv, opt_str ) ) != -1 ) {
			switch ( opt ) {
				case 'c':
					if ( appconfig_load( optarg ) < 0 ) {
						return -1;
					}
					else {
						// 初始化log
						const char* log_config_file = appconfig_get_str( "application.log_config_file" );
						if ( log_init( log_config_file ) < 0 ) {
							return -1;
						}
						config_loaded = 1;
					}
					break;
				case 'D':
					dont_fork = 1;
					break;
				case 'V':
				case 'v':
					fprintf( stderr, "x-monitor Version: %d.%d", XMonitor_VERSION_MAJOR, XMonitor_VERSION_MINOR );
					return 0;
				case 'h':
				default:
					help();
					return 0;
			}
		}
	}

	if ( !config_loaded ) {
		help();
	}

	// 信号初始化
	signals_block();
	signals_init();

	// 编程守护进程
	pid_file = appconfig_get_str( "application.pid_file" );
	const char * user = appconfig_get_str( "application.run_as_user" );
	become_daemon( dont_fork, pid_file, user );

	info( "---start mypopen running pid: %d---", getpid() );

	// 初始化插件管理
	pluginsd_main( NULL );

	// 解除信号阻塞
	signals_unblock();
	// 信号处理
	signals_handle( main_cleanup_and_exit );

	// const char* cmd = appconfig_get_str( "plugins.timer_shell" );
	// if ( cmd == NULL ) {
	// 	error( "plugins.timer_shell is not set." );
	// 	return 1;
	// }

	// FILE* child_fp = mypopen( cmd, &child_pid );
	// if ( unlikely( !child_fp ) ) {
	// 	error( "Cannot popen(\"%s\", \"r\").", cmd );
	// 	return 0;
	// }

	// debug( "connected to '%s' running on pid %d", cmd, child_pid );

	// while ( 1 ) {
	// 	if ( fgets( buf, BUF_SIZE, child_fp ) == NULL ) {
	// 		if ( feof( child_fp ) ) {
	// 			info( "fgets() return EOF." );
	// 			break;
	// 		}
	// 		else if ( ferror( child_fp ) ) {
	// 			info( "fgets() return error." );
	// 			break;
	// 		}
	// 		else {
	// 			info( "fgets() return unknown." );
	// 		}
	// 	}
	// 	buf[strlen(buf) - 1] = '\0';
	// 	info( "recv: [%s]", buf );
	// }

	// info( "'%s' (pid %d) disconnected.", cmd, child_pid );

	// killpid( child_pid );

	// int32_t child_worker_ret_code = mypclose( child_fp, child_pid );
	// if ( likely( child_worker_ret_code == 0 ) ) {
	// 	info( "child worker exit normally." );
	// }
	// else {
	// 	error( "child worker exit abnormally." );
	// }

	return 0;
}