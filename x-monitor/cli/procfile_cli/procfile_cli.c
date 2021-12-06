/*
 * @Author: CALM.WU 
 * @Date: 2021-12-06 11:13:59 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-12-06 15:10:31
 */

#include "utils/common.h"
#include "utils/procfile.h"
#include "utils/log.h"

const char *seps = " \t:";

int32_t main(int32_t argc, char **argv) {
    int32_t ret = 0;

    char *log_cfg = argv[1];
    char *proc_file = argv[2];

    if(log_init(log_cfg, "procfile_cli") != 0) {
        fprintf(stderr, "log init failed\n");
        return -1;
    }

    fprintf(stderr, "open proc file: %s", proc_file);

    struct proc_file *pf = procfile_open(proc_file, seps, PROCFILE_FLAG_DEFAULT);
    if(pf == NULL) {
        fprintf(stderr, "open proc file failed\n");
        return -1;
    }

    pf = procfile_readall(pf);
    
    procfile_print(pf);
    
    procfile_close(pf);

    log_fini();
    
    return ret;
}