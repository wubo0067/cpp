/*
 * @Author: CALM.WU 
 * @Date: 2021-10-22 16:28:26 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-10-22 17:12:41
 */

#include "utils/common.h"
#include "routine.h"

static xmonitor_static_routine_t routine;

__attribute__((constructor)) static void register_myself() {
    fprintf(stderr, "init_exitflag\n");
    routine.exit_flag = 99;
}

int32_t main(int argc, char const *argv[])
{
    fprintf(stderr, "static variable main routine.exit_flag: %d\n", routine.exit_flag);
    return 0;
}
