/*
 * @Author: CALM.WU
 * @Date: 2021-10-12 10:44:47
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-10-12 14:45:33
 */

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "log.h"
#include "popen.h"

#define BUF_SIZE 1024

const char* const __cmd = "../timer.sh";

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

int32_t main( int32_t argc, char* argv[] ) {
	char buf[BUF_SIZE];
	pid_t child_pid = 0;

	info( "---start mypopen running pid: %d---", getpid() );

	FILE* child_fp = mypopen( __cmd, &child_pid );
	if ( unlikely( !child_fp ) ) {
		error( "Cannot popen(\"%s\", \"r\").", __cmd );
		return 0;
	}

	debug( "connected to '%s' running on pid %d", __cmd, child_pid );

	while ( 1 ) {
		if ( fgets( buf, BUF_SIZE, child_fp ) == NULL ) {
			if ( feof( child_fp ) ) {
				info( "fgets() return EOF." );
				break;
			}
			else if ( ferror( child_fp ) ) {
				info( "fgets() return error." );
				break;
			}
			else {
				info( "fgets() return unknown." );
			}
		}
		info( "buf: %s", buf );
	}

	info( "'%s' (pid %d) disconnected.", __cmd, child_pid );

	killpid( child_pid );

	int32_t child_worker_ret_code = mypclose( child_fp, child_pid );
	if ( likely( child_worker_ret_code == 0 ) ) {
		info( "child worker exit normally." );
	}
	else {
		error( "child worker exit abnormally." );
	}

	return 0;
}