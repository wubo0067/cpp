/*
 * @Author: CALM.WU
 * @Date: 2021-10-15 10:20:51
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-10-15 17:47:27
 */

#include "daemon.h"
#include "compiler.h"
#include "log.h"
#include "files.h"

char pid_file[FILENAME_MAX + 1] = { 0 };

const int32_t process_nice_level = 19;
const int32_t process_oom_score  = 1000;

// 设置oom_socre_adj为-1000，表示禁止oom killer杀死该进程
static void oom_score_adj( void ) {
	int64_t old_oom_score = 0;
	int32_t ret           = 0;

	ret = read_file_to_int64( "/proc/self/oom_score_adj", &old_oom_score );
	if ( unlikely( ret < 0 ) ) {
		error( "read /proc/self/oom_score_adj failed, ret: %d", ret );
		return;
	}

	if ( old_oom_score == process_oom_score ) {
		info( "oom_score_adj is already %d", process_oom_score );
		return;
	}

	ret = write_int64_to_file( "/proc/self/oom_score_adj", process_oom_score );
	if ( unlikely( ret < 0 ) ) {
		error( "failed to adjust Out-Of-Memory (OOM) score to %d. run with %d, ret: %d", process_oom_score,
		    old_oom_score, ret );
		return;
	}

	info( "adjust Out-Of-Memory (OOM) score from %d to %d.", old_oom_score, process_oom_score );
	return;
}

static inline void set_process_nice_level() {
	if ( nice( process_nice_level ) == -1 ) {
		error( "Cannot set CPU nice level to %d.", process_nice_level );
	}
	else {
		debug( "Set nice level to %d.", process_nice_level );
	}
}

int32_t mk_daemon( int32_t dont_fork, const char* user ) {
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
		}
		else {
			error( "Cannot open pidfile '%s'.", pid_file );
		}
	}

	// file mode creation mask
	umask( 0007 );

	// adjust my Out-Of-Memory score
	oom_score_adj();

	// 调整进程调度优先级
	set_process_nice_level();

	if ( user && *user ) {
		struct passwd* pw = getpwnam( user );

		if ( pidfd != -1 ) {
			close( pidfd );
		}
	}

	return 0;
}