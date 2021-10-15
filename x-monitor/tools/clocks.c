/*
 * @Author: CALM.WU 
 * @Date: 2021-10-14 14:34:44 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-10-15 10:55:56
 */

#include "clocks.h"
#include "compiler.h"

static inline time_t now_sec( clockid_t clk_id ) {
	struct timespec ts;
	if ( unlikely( clock_gettime( clk_id, &ts ) == -1 ) ) {
		return 0;
	}
	return ts.tv_sec;
}

inline time_t now_realtime_sec( void ) { return now_sec( CLOCK_REALTIME ); }