/*
 * @Author: CALM.WU 
 * @Date: 2021-12-06 11:13:59 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-12-06 14:32:05
 */

#include "utils/common.h"
#include "utils/procfile.h"
#include "utils/log.h"

int32_t main(int32_t argc, char **argv) {
    int32_t ret = 0;

    char *log_cfg = argv[1];
    char *proc_file = argv[2];

    if(log_init(log_cfg, "procfile_cli") != 0) {
        fprintf(stderr, "log init failed\n");
        return -1;
    }

    debug("open proc file: %s", proc_file);

    struct proc_file *pf = procfile_open(proc_file, " \t", PROCFILE_FLAG_DEFAULT);
    if(pf == NULL) {
        error("open proc file failed");
        return -1;
    }

    procfile_readall(pf);
    
    procfile_print(pf);
    
    procfile_close(pf);

    log_fini();
    
    return ret;
}