/*
 * @Author: CALM.WU
 * @Date: 2021-10-15 10:20:51
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-10-15 14:09:04
 */

#include "daemon.h"

char pid_file[FILENAME_MAX + 1] = { 0 };

int32_t daemon( int32_t dont_fork, const char* user ) {
	if ( !dont_fork ) {
		int32_t i = fork();
		if ( i == -1 ) {
			perror( "cannot fork" );
			exit( 0 - errno );
		}

		if ( i != 0 ) {
			// parent exit
			exit( 0 );
		}

		// become session leader
		if ( setsid() < 0 ) {
			perror( "cannot become session leader" );
			exit( 0 - errno );
		}

		// fork again
		i = fork();
		if ( i == -1 ) {
			perror( "cannot fork" );
			exit( 0 - errno );
		}

		if ( i != 0 ) {
			// parent exit
			exit( 0 );
		}
	}

	// 生成pid文件
	int32_t pidfd = -1;
	if ( pid_file[0] != '\0' ) {
		pidfd = open( pid_file, O_RDWR | O_CREAT, 0644 );
		if ( pidfd >= 0 ) {
			char pid_str[32] = { 0 };
			sprintf( pid_str, "%d", getpid() );
			if ( write( pidfd, pid_str, strlen( pid_str ) ) <= 0 ) {
				error( "Cannot write pidfile '%s'.", pid_file );
			}
		} else {
            error( "Cannot open pidfile '%s'.", pid_file );
        }
	}

    // file mode creation mask
    umask( 0007 );

    if(user && *user) {
        struct passwd *pw = getpwnam(user);

    if(pidfd != -1) {
        close(pidfd);
    }

	return 0;
}