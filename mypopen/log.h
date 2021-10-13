/*
 * @Author: CALM.WU
 * @Date: 2021-10-12 11:15:44
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-10-12 15:08:39
 */
#pragma once

#include <time.h>

#define unlikely( x ) __builtin_expect( !!( x ), 0 )
#define likely( x ) __builtin_expect( !!( x ), 1 )

#ifdef HAVE_FUNC_ATTRIBUTE_FORMAT
#	define PRINTFLIKE( f, a ) __attribute__( ( format( __printf__, f, a ) ) )
#else
#	define PRINTFLIKE( f, a )
#endif

// const char * 是限制指针内容，*后面的const是限定变量的，这样才能赋予内部链接属性
// extern const联合修饰时，extern会压制const内部链接属性

enum log_level
{
	LOG_DEBUG = 0,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR,
	LOG_FATAL,
	LOG_LEVEL_MAX
};

#define debug( args... ) log_print( LOG_DEBUG, __FILE__, __FUNCTION__, __LINE__, ##args )
#define info( args... ) log_print( LOG_INFO, __FILE__, __FUNCTION__, __LINE__, ##args )
#define warn( args... ) log_print( LOG_WARN, __FILE__, __FUNCTION__, __LINE__, ##args )
#define error( args... ) log_print( LOG_ERROR, __FILE__, __FUNCTION__, __LINE__, ##args )
#define fatal( args... ) log_print( LOG_FATAL, __FILE__, __FUNCTION__, __LINE__, ##args )

extern void log_print(
    enum log_level level, const char* file, const char* function, const unsigned long line, const char* fmt, ... )
    PRINTFLIKE( 4, 5 );
extern time_t now_realtime_sec( void );
