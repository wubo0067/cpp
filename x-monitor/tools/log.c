/*
 * @Author: CALM.WU
 * @Date: 2021-10-12 11:15:36
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-10-15 11:20:07
 */


#include "log.h"
#include "compiler.h"
#include "clocks.h"

#define LOG_DATE_LENGTH 26

#define DLEVEL_STR "DEBUG"
#define ILEVEL_STR "INFO"
#define WLEVEL_STR "WARN"
#define ELEVEL_STR "ERROR"
#define FLEVEL_STR "FATAL"
#define ULEVEL_STR "UNKNOWN"

typedef struct {
	const char* const name;
	enum log_level level;
} __log_name_level_t;

static const __log_name_level_t log_name_levels[] = {
	{ DLEVEL_STR, LOG_LEVEL_DEBUG },
	{ ILEVEL_STR, LOG_LEVEL_INFO },
	{ WLEVEL_STR, LOG_LEVEL_WARN },
	{ ELEVEL_STR, LOG_LEVEL_ERROR },
	{ FLEVEL_STR, LOG_LEVEL_FATAL },
};

static enum log_level __log_level  = LOG_LEVEL_DEBUG;
static pthread_mutex_t __log_mutex = PTHREAD_MUTEX_INITIALIZER;

static inline void __log_lock( void ) { pthread_mutex_lock( &__log_mutex ); }

static inline void __log_unlock( void ) { pthread_mutex_unlock( &__log_mutex ); }

static inline const char* get_name_by_log_level( int level ) {
	int i;
	for ( i = 0; i < sizeof( log_name_levels ) / sizeof( __log_name_level_t ); i++ ) {
		if ( log_name_levels[i].level == level ) {
			return log_name_levels[i].name;
		}
	}
	return ULEVEL_STR;
}

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

	if ( unlikely( level > __log_level ) )
		return;

	log_date( date, LOG_DATE_LENGTH );

	level_name = get_name_by_log_level( level );

	__log_lock();

	va_start( args, fmt );
	printf( "%s: %s %-5s %s:%lu@%s: ", date, "myopen", level_name, file, line, function );
	vprintf( fmt, args );
	va_end( args );
	putchar( '\n' );

	fflush( stdout );

	__log_unlock();
}

void log_set_level( enum log_level level ) { __log_level = level; }