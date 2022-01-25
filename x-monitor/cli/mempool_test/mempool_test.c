/*
 * @Author: CALM.WU
 * @Date: 2022-01-25 11:02:43
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2022-01-25 11:32:09
 */

#include "utils/common.h"
#include "utils/log.h"
#include "utils/mempool.h"

struct __test {
    int32_t a;
    int32_t b;
    int32_t c;
};

static void test_malloc_1(struct xm_mempool_s *xmp) {
    void *mem = xm_mempool_malloc(xmp);

    xm_print_mempool_block_info_by_pointer(xmp, mem);

    xm_mempool_free(xmp, mem);

    xm_print_mempool_block_info_by_pointer(xmp, mem);
}

int32_t main(int32_t argc, char **argv) {
    if (log_init("../cli/mempool_test/log.cfg", "mempool_test") != 0) {
        fprintf(stderr, "log init failed\n");
        return -1;
    }

    debug("mempool_test");

    struct xm_mempool_s *xmp = xm_mempool_init(sizeof(struct __test), 10, 10);

    xm_print_mempool_info(xmp);

    test_malloc_1(xmp);

    xm_mempool_fini(xmp);

    log_fini();

    return 0;
}