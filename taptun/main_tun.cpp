/*
 * @Author: calmwu
 * @Date: 2020-12-27 12:59:33
 * @Last Modified by: calmwu
 * @Last Modified time: 2020-12-27 13:00:38
 */

// https://github.com/LaKabane/libtuntap
// https://github.com/gregnietsky/simpletun/blob/master/simpletun.c
// https://blog.csdn.net/xxb249/article/details/86690067 C语言创建tap设备并且设置ip

#include <errno.h>
#include <fcntl.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

const char *cloneDev = "/dev/net/tun";

//
void dbg_print(const char *msg, ...) {
    va_list argp;

    va_start(argp, msg);
    vfprintf(stderr, msg, argp);
    va_end(argp);
}

int32_t tun_alloc(char *dev, int32_t flags) {
    if (nullptr == dev) {
        dbg_print("tun_alloc dev is NULL\n");
        return -1;
    }

    struct ifreq ifr;
    int32_t fd, err;

    if ((fd = open(cloneDev, O_RDWR)) < 0) {
        dbg_print("Opening %s failed, err:%s\n", cloneDev, strerror(errno));
        return fd;
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = flags;

    // 如果设置了接口名字
    if (*dev != '\0') {
        strncpy(ifr.ifr_name, dev, IF_NAMESIZE);
    }

    if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0) {
        dbg_print("ioctl TUNSETIFF failed, err:%s\n", strerror(errno));
        close(fd);
        return err;
    }

    // 如果没有指定接口名字，系统会返回默认的名字
    strcpy(dev, ifr.ifr_name);

    dbg_print("Open tun/tap device: %s for reading...\n", ifr.ifr_name);

    return fd;
}

void testAllocMultiTunDevs(int32_t tap_dev_count) {
    int32_t tun_fd;
    char if_name[IF_NAMESIZE] = {0};

    for (int32_t i = 0; i < tap_dev_count; i++) {
        /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
         *        IFF_TAP   - TAP device
         *        IFF_NO_PI - Do not provide packet information
         */
        sprintf(if_name, "calm-tun%d", i);
        tun_fd = tun_alloc(if_name, IFF_TUN | IFF_NO_PI);
        if (tun_fd < 0) {
            dbg_print("Error connecting to tun/tap interface:[%s]!\n", if_name);
            exit(-1);
        }

        dbg_print("Successfully connected to interface:[%s]\n", if_name);
    }
}

int32_t main(int32_t argc, char **argv) {
    testAllocMultiTunDevs(3);

    sleep(3);

    return 0;
}