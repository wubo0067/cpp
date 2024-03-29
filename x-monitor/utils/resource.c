/*
 * @Author: CALM.WU
 * @Date: 2021-11-03 11:51:34
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2022-01-13 16:53:51
 */

#include "common.h"
#include "log.h"
#include "procfile.h"
#include "resource.h"

int32_t processors = 1;

static const char __no_user[] = "";

int32_t bump_memlock_rlimit(void) {
    struct rlimit rlim_new = {
        .rlim_cur = RLIM_INFINITY,
        .rlim_max = RLIM_INFINITY,
    };

    return setrlimit(RLIMIT_MEMLOCK, &rlim_new);
}

const char *get_username(uid_t uid) {
    struct passwd *pwd = getpwuid(uid);
    if (pwd == NULL) {
        return __no_user;
    }
    return pwd->pw_name;
}

int32_t get_system_cpus() {
    // return sysconf(_SC_NPROCESSORS_ONLN); 加强移植性

    struct proc_file *pf_stat = procfile_open("/proc/stat", NULL, PROCFILE_FLAG_DEFAULT);
    if (unlikely(!pf_stat)) {
        error("Cannot open /proc/stat. Assuming system has %d processors. error: %s", processors,
              strerror(errno));
        return processors;
    }

    pf_stat = procfile_readall(pf_stat);
    if (unlikely(!pf_stat)) {
        error("Cannot read /proc/stat. Assuming system has %d processors.", processors);
        return processors;
    }

    processors = 0;

    for (int32_t index = 0; index < procfile_lines(pf_stat); index++) {
        if (!procfile_linewords(pf_stat, index)) {
            continue;
        }

        if (strncmp(procfile_lineword(pf_stat, index, 0), "cpu", 3) == 0) {
            processors++;
        }
    }

    processors--;
    if (processors < 1) {
        processors = 1;
    }

    procfile_close(pf_stat);

    debug("System has %d processors.", processors);

    return processors;
}