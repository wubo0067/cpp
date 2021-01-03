/*
 * @Author: calmwu
 * @Date: 2021-01-03 19:42:18
 * @Last Modified by:   calmwu
 * @Last Modified time: 2021-01-03 19:42:18
 */

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int32_t main(int32_t argc, char* argv[]) {
    char buf[128] = {0};
    int fd = open("/proc/calmdev", O_RDWR);
    read(fd, buf, 128);
    puts(buf);

    lseek(fd, 0 , SEEK_SET);
    write(fd, "33 4", 5);

    lseek(fd, 0 , SEEK_SET);
    read(fd, buf, 128);
    puts(buf);
}
