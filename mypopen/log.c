/*
 * @Author: CALM.WU
 * @Date: 2021-10-12 11:15:36
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-10-12 15:07:38
 */

#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define LOG_DATE_LENGTH 26

#define DLEVEL "DEBUG"
#define ILEVEL "INFO"
#define WLEVEL "WARN"
#define ELEVEL "ERROR"
#define FLEVEL "FATAL"
#define ULEVEL "UNKNOWN"

typedef struct {
	const char* const name;
	int level;
} __log_name_level_t;

static const __log_name_level_t log_name_levels[] = {
	{ DLEVEL, LOG_DEBUG },
	{ ILEVEL, LOG_INFO },
	{ WLEVEL, LOG_WARN },
	{ ELEVEL, LOG_ERROR },
	{ FLEVEL, LOG_FATAL },
};

static inline const char* get_name_by_log_level( int level ) {
	int i;
	for ( i = 0; i < sizeof( log_name_levels ) / sizeof( __log_name_level_t ); i++ ) {
		if ( log_name_levels[i].level == level ) {
			return log_name_levels[i].name;
		}
	}
	return ULEVEL;
}

static inline time_t now_sec( clockid_t clk_id ) {
	struct timespec ts;
	if ( unlikely( clock_gettime( clk_id, &ts ) == -1 ) ) {
		return 0;
	}
	return ts.tv_sec;
}

inline time_t now_realtime_sec( void ) { return now_sec( CLOCK_REALTIME ); }

static inline void log_date( char* buffer, size_t len ) {
	if ( unlikely( !buffer || !len ) )
		return;

	time_t t;
	struct tm *tmp, tmbuf;

	t   = now_realtime_sec();
	tmp = localtime_r( &t, &tmbuf );

	if ( tmp == NULL ) {
		buffer[0] = '\0';
		return;
	}

	if ( unlikely( strftime( buffer, len, "%Y-%m-%d %H:%M:%S", tmp ) == 0 ) )
		buffer[0] = '\0';

	buffer[len - 1] = '\0';
}

void log_print(
    enum log_level level, const char* file, const char* function, const unsigned long line, const char* fmt, ... ) {
	va_list args;

	const char* level_name = NULL;
	char date[LOG_DATE_LENGTH];

	log_date( date, LOG_DATE_LENGTH );

	level_name = get_name_by_log_level( level );

	va_start( args, fmt );
	printf( "%s: %s %-5s %s:%d@%s: ", date, "myopen", level_name, file, line, function );
	vprintf( fmt, args );
	va_end( args );
	putchar( '\n' );

	fflush( stdout );
}