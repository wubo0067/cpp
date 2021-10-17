/*
 * @Author: CALM.WU
 * @Date: 2021-10-15 10:26:53
 * @Last Modified by:   CALM.WU
 * @Last Modified time: 2021-10-15 10:26:53
 */

#include "signals.h"
#include "common.h"
#include "compiler.h"
#include "log.h"

typedef enum
{
	E_SIGNAL_IGNORE,
	E_SIGNAL_EXIT_CLEANLY,
	E_SIGNAL_SAVE_DATABASE,
	E_SIGNAL_REOPEN_LOGS,
	E_SIGNAL_FATAL,
	E_SIGNAL_CHILD,
} __signal_action_t;

typedef struct {
	int32_t signo;            // 信号
	const char* signame;      // 信号名称
	uint32_t receive_count;   //
	__signal_action_t action; //
} __signals_waiting_t;

static __signals_waiting_t signal_waiting_list[] = {
	{ SIGPIPE, "SIGPIPE", 0, E_SIGNAL_IGNORE }, { SIGINT, "SIGINT", 0, E_SIGNAL_EXIT_CLEANLY },
	{ SIGTERM, "SIGTERM", 0, E_SIGNAL_EXIT_CLEANLY }, { SIGTERM, "SIGTERM", 0, E_SIGNAL_EXIT_CLEANLY },
	{ SIGQUIT, "SIGQUIT", 0, E_SIGNAL_EXIT_CLEANLY }, { SIGBUS, "SIGBUS", 0, E_SIGNAL_FATAL },
	{ SIGCHLD, "SIGCHLD", 0, E_SIGNAL_CHILD }
	//{ SIGUSR1, "SIGUSR1", 0, NETDATA_SIGNAL_SAVE_DATABASE },
	//{ SIGUSR2, "SIGUSR2", 0, NETDATA_SIGNAL_RELOAD_HEALTH },
};

static void signal_handler( int signo ) {
	// find the entry in the list
	int i;
	for ( i = 0; i < sizeof( signal_waiting_list ) / sizeof( __signals_waiting_t ); i++ ) {
		if ( unlikely( signal_waiting_list[i].signo == signo ) ) {
			// 信号接收计数
			signal_waiting_list[i].receive_count++;

			if ( signal_waiting_list[i].action == E_SIGNAL_FATAL ) {
				char buffer[200 + 1];
				snprintf(
				    buffer, 200, "\nSIGNAL HANDLER: received: %s. Oops! This is bad!\n", signal_waiting_list[i].signame );
				write( STDERR_FILENO, buffer, strlen( buffer ) );
			}
			return;
		}
	}
}

void signals_init( void ) {
	struct sigaction sa;
	sa.sa_flags = 0; // sa_flags 字段指定对信号进行处理的各个选项。例如SA_NOCLDWAIT

	sigfillset( &sa.sa_mask ); // 调用信号处理函数时，要屏蔽所有的信号。

	int32_t i = 0;
	for ( i = 0; i < sizeof( signal_waiting_list ) / sizeof( __signals_waiting_t ); i++ ) {
		switch ( signal_waiting_list[i].action ) {
			case E_SIGNAL_IGNORE:
				sa.sa_handler = SIG_IGN;
				break;
			default:
				sa.sa_handler = signal_handler;
				break;
		}
		// 注册信号处理函数
		if ( sigaction( signal_waiting_list[i].signo, &sa, NULL ) == -1 ) {
			error(
			    "Cannot set signal handler for %s (%d)", signal_waiting_list[i].signame, signal_waiting_list[i].signo );
		}
	}
}

void signals_handle( void ) {
	while ( 1 ) {
		if ( pause() == -1 && errno == EINTR ) {
			int32_t found = 1;

			while ( found ) {
				found = 0;
			}
		}
	}
}