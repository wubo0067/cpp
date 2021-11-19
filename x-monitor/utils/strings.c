/*
 * @Author: CALM.WU 
 * @Date: 2021-11-19 10:46:05 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-11-19 13:59:49
 */

#include "strings.h"

int32_t str_split_to_nums(const char *str, const char *delim, uint64_t *nums,
                          uint16_t nums_max_size)
{
    int32_t  count = 0;
    uint64_t num   = 0;

    if (str == NULL || delim == NULL || nums == NULL || strlen(str) == 0 ||
        strlen(delim) == 0) {
        return -EINVAL;
    }

    for (char *tok = strtok((char * restrict)str, delim); tok != NULL && count < nums_max_size;
         tok       = strtok(NULL, delim), count++) {
        num         = strtoull(tok, NULL, 10);
        nums[count] = num;
    }

    return count;
}