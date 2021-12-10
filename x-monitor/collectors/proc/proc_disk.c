/*
 * @Author: CALM.WU 
 * @Date: 2021-11-30 14:59:07 
 * @Last Modified by:   CALM.WU 
 * @Last Modified time: 2021-11-30 14:59:07 
 */

#include "proc_disk.h"
#include "utils/strings.h"
#include "utils/procfile.h"
#include "utils/consts.h"

static const char *      __proc_disk_filename = "/proc/diskstats";
static struct proc_file *__pf_diskstats = NULL;

struct io_stats {
    /* 读取的扇区数量 */
    uint64_t rd_sectors;
    /* 写入的扇区数量 */
    uint64_t wr_sectors;
    /* # of sectors discarded */
    uint64_t dc_sectors;
    /* (rd_ios)读操作的次数 */
    uint64_t rd_ios;
    /* 合并读操作的次数。如果两个读操作读取相邻的数据块时，可以被合并成一个，以提高效率。合并的操作通常是I/O scheduler（也叫elevator）负责的 */
    uint64_t rd_merges;
    /* 写操作的次数 */
    uint64_t wr_ios;
    /* 合并写操作的次数 */
    uint64_t wr_merges;
    /* # of discard operations issued to the device */
    uint64_t dc_ios;
    /* # of discard requests merged */
    uint64_t dc_merges;
    /* # of flush requests issued to the device */
    uint64_t fl_ios;
    /* 读操作消耗的时间（以毫秒为单位）。每个读操作从__make_request()开始计时，到end_that_request_last()为止，包括了在队列中等待的时间 */
    uint64_t rd_ticks;
    /* 写操作消耗的时间（以毫秒为单位） */
    uint64_t wr_ticks;
    /* Time of discard requests in queue */
    uint64_t dc_ticks;
    /* Time of flush requests in queue */
    uint64_t fl_ticks;
    /* # of I/Os in progress 
    当前正在进行的I/O数量,唯一应该归零的字段。随着请求的增加而增加,提供给相应的结构请求队列，并在完成时递减。*/
    uint64_t ios_pgr;
    /* 执行I/O所花费的毫秒数，只要排队的字段不为零，此字段就会增加。 */
    uint64_t tot_ticks;
    /* # of ticks requests spent in queue 
    number of milliseconds spent doing I/Os. This field is incremented at each I/O start, 
    I/O completion, I/O merge, or read of these stats by the number of I/Os in progress (field 9) 
    times the number of milliseconds spent doing I/O since the last update of this field. 
    This can provide an easy measure of both I/O completion time and the backlog that may be accumulating.
    */
    uint64_t rq_ticks;
};

struct io_device {
    char device_name[MAX_NAME_LEN + 1]; //

    uint32_t       major;
    uint32_t       minor;
    uint32_t       device_hash;
    enum disk_type dev_tp;

    struct io_stats stats[2];
    uint32_t        course_prev_stats;
    uint32_t        course_prev_stats;

    struct io_device *next;
};

//
static const int32_t __kernel_sector_size = 512;
static const double  __kernel_sector_kb = (double)__kernel_sector_size / 1024.0;
//
static struct io_device *__iodev_list = NULL;

static enum disk_type __get_device_type(const char *dev_name, uint32_t major,
                                        uint32_t minor) {
    enum disk_type dt = DISK_TYPE_UNKNOWN;

    char buffer[MAX_NAME_LEN + 1];
    snprintf(buffer, MAX_NAME_LEN, "/sys/block/%s", dev_name);
    if (likely(access(buffer, R_OK) == 0)) {
        // assign it here, but it will be overwritten if it is not a physical disk
        dt = DISK_TYPE_PHYSICAL;
    } else {
        snprintf(buffer, MAX_NAME_LEN, "/sys/dev/block/%lu:%lu/partition",
                 major, minor);
        if (likely(access(buffer, R_OK) == 0)) {
            dt = DISK_TYPE_PARTITION;
        } else {
            snprintfz(buffer, FILENAME_MAX, "/sys/dev/block/%lu:%lu/slaves",
                      major, minor);
            DIR *dirp = opendir(buffer);
            if (likely(dirp != NULL)) {
                struct dirent *dp;
                while ((dp = readdir(dirp))) {
                    // . and .. are also files in empty folders.
                    if (unlikely(strcmp(dp->d_name, ".") == 0 ||
                                 strcmp(dp->d_name, "..") == 0)) {
                        continue;
                    }

                    dt = DISK_TYPE_VIRTUAL;
                    // Stop the loop after we found one file.
                    break;
                }
                if (unlikely(closedir(dirp) == -1))
                    error("Unable to close dir %s", buffer);
            }
        }
    }

    return dt;
}

static struct io_device *__get_device(uint32_t major, uint32_t minor,
                                      char *device_name) {
    struct io_device *dev = NULL;
    int32_t           index = 0;

    // 计算hash值
    uint32_t hash = bkrd_hash(device_name, strlen(device_name));

    // 查找磁盘设备，看是否已经存在
    for (dev = __iodev_list; dev != NULL; dev = dev->next) {
        if (dev->major == major && dev->minor == minor &&
            dev->device_hash == hash) {
            return dev;
        }
    }

    dev = (struct io_device *)calloc(1, sizeof(struct io_device));
    if (unlikely(!dev)) {
        exit(-1);
    }

    strncpy(dev->disk_name, device_name, MAX_NAME_LEN);
    dev->major = major;
    dev->minor = minor;
    dev->device_hash = hash;
    dev->dev_tp = __get_device_type(device_name, major, minor);
    dev->course_prev_stats = dev->course_prev_stats = 0;

    if (unlikely(!__iodev_list)) {
        __iodev_list = dev;
    } else {
        // 添加在末尾
        struct io_device *last;
        for (last = __iodev_list; last->next != NULL; last = last->next)
            ;
        last->next = dev;
    }

    return d;
}

int32_t collector_proc_diskstats(int32_t update_every, usec_t dt) {
    if (unlikely(!__pf_diskstats)) {
        __pf_diskstats =
            procfile_open(__proc_disk_filename, " \t", PROCFILE_FLAG_DEFAULT);
        if (unlikely(!__pf_diskstats)) {
            error("Cannot open /proc/diskstats");
            return -1;
        }
    }

    __pf_diskstats = procfile_readall(__pf_diskstats);
    if (unlikely(!__pf_diskstats)) {
        error("Cannot read /proc/diskstats");
        return -1;
    }

    size_t pf_diskstats_lines = procfile_lines(__pf_diskstats), l;

    int64_t system_read_kb = 0, system_write_kb = 0;

    for (l = 0; l < pf_diskstats_lines; l++) {
        // 每一行都是一个磁盘设备
        char *dev_name;
        // 设备的主id，辅id
        uint32_t major, minor;

        struct io_stats curr_diskstats;
        memset(&curr_diskstats, 0, sizeof(struct io_stats));

        size_t pf_diskstats_line_words = procfile_linewords(__pf_diskstats, l);
        if (unlikely(pf_diskstats_line_words < 14)) {
            error("Cannot read /proc/diskstats: line %zu is too short.", l);
            continue;
        }

        major = str2uint32_t(procfile_lineword(__pf_diskstats, l, 0));
        minor = str2uint32_t(procfile_lineword(__pf_diskstats, l, 1));
        dev_name = procfile_lineword(__pf_diskstats, l, 2);

        debug("diskstats: major=%u, minor=%u, dev_name=%s", major, minor,
              dev_name);

        struct io_device *dev = __get_device(dev_name, major, minor);
        struct io_stats * curr_diskstats = &dev->stats[dev->course_prev_stats];

        curr_diskstats->rd_ios =
            str2uint64_t(procfile_lineword(__pf_diskstats, l, 3));
        curr_diskstats->rw_ios =
            str2uint64_t(procfile_lineword(__pf_diskstats, l, 7));

        curr_diskstats->rd_merges =
            str2uint64_t(procfile_lineword(__pf_diskstats, l, 4));
        curr_diskstats->wr_merges =
            str2uint64_t(procfile_lineword(__pf_diskstats, l, 8));

        curr_diskstats->rd_sectors =
            str2uint64_t(procfile_lineword(__pf_diskstats, l, 5));
        curr_diskstats->wr_sectors =
            str2uint64_t(procfile_lineword(__pf_diskstats, l, 9));

        curr_diskstats->rd_ticks =
            str2uint64_t(procfile_lineword(__pf_diskstats, l, 6));
        curr_diskstats->wr_ticks =
            str2uint64_t(procfile_lineword(__pf_diskstats, l, 10));

        curr_diskstats->ios_pgr =
            str2uint64_t(procfile_lineword(__pf_diskstats, l, 11));

        curr_diskstats->tot_ticks =
            str2uint64_t(procfile_lineword(__pf_diskstats, l, 12));

        curr_diskstats->rq_ticks =
            str2uint64_t(procfile_lineword(__pf_diskstats, l, 13));

        debug(
            "diskstats[%d:%d]: rd_ios=%llu, rw_ios=%llu, rd_merges=%llu, wr_merges=%llu, rd_sectors=%llu, wr_sectors=%llu, rd_ticks=%llu, wr_ticks=%llu, ios_pgr=%llu, tot_ticks=%llu, rq_ticks=%llu",
            dev->course_prev_stats, dev->course_prev_stats,
            curr_diskstats->rd_ios, curr_diskstats->rw_ios,
            curr_diskstats->rd_merges, curr_diskstats->wr_merges,
            curr_diskstats->rd_sectors, curr_diskstats->wr_sectors,
            curr_diskstats->rd_ticks, curr_diskstats->wr_ticks,
            curr_diskstats->ios_pgr, curr_diskstats->tot_ticks,
            curr_diskstats->rq_ticks);

        if (unlikely(0 == dev->course_prev_stats &&
                     0 == dev->course_prev_stats)) {
            continue;
        }

        //--------------------------------------------------------------------
        // do performance metrics
        struct io_stats *prev_diskstats = &dev->stats[dev->course_prev_stats];

        double dt_sec = double((double)dt / USEC_PER_SEC);
        // IO每秒读次数
        double rd_ios_per_sec =
            ((double)(curr_diskstats->rd_ios - prev_diskstats->rd_ios)) /
            dt_sec;
        // IO每秒写次数
        double rw_ios_per_sec =
            ((double)(curr_diskstats->rw_ios - prev_diskstats->rw_ios)) /
            dt_sec;

        // 每秒读取字节数
        double rd_kb_per_sec =
            ((double)(curr_diskstats->rd_sectors - prev_diskstats->rd_sectors) / 2.0 / dt_sec;
        // 每秒写入字节数
        double wr_kb_per_sec =
            ((double)(curr_diskstats->wr_sectors - prev_diskstats->wr_sectors) / 2.0 / dt_sec;

        // 每秒合并读次数 rrqm/s
        double rd_merges_per_sec =
            ((double)(curr_diskstats->rd_merges - prev_diskstats->rd_merges)) /
            dt_sec;
        // 每秒合并写次数 wrqm/s
        double wr_merges_per_sec =
            ((double)(curr_diskstats->wr_merges - prev_diskstats->wr_merges)) /
            dt_sec;

        // 每个读操作的耗时（毫秒）
        double r_await = (curr_diskstats->rd_ios - prev_diskstats->rd_ios) ?
                                   (curr_diskstats->rd_ticks - prev_diskstats->rd_ticks) /
                                    ((double)(curr_diskstats->rd_ios - prev_diskstats->rd_ios)) : 0.0;

        // 每个写操作的耗时（毫秒
        double w_await = (curr_diskstats->wr_ios - prev_diskstats->wr_ios) ?
                                   (curr_diskstats->wr_ticks - prev_diskstats->wr_ticks) /
                                    ((double)(curr_diskstats->wr_ios - prev_diskstats->wr_ios)) : 0.0;

        // --------------------------------------------------------------------
        dev->course_prev_stats = dev->course_prev_stats;
        dev->course_prev_stats = (dev->course_prev_stats + 1) % 2;
    }

    return 0;
}