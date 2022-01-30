/*
 * @Author: CALM.WU
 * @Date: 2022-01-26 17:00:40
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2022-01-26 17:01:11
 */

// 取得物理内存信息（相关文件/proc/meminfo）
// https://blog.51cto.com/u_13434336/2488970

#include "plugin_proc.h"

#include "prometheus-client-c/prom.h"

#include "utils/compiler.h"
#include "utils/consts.h"
#include "utils/log.h"
#include "utils/procfile.h"
#include "utils/strings.h"

#include "appconfig/appconfig.h"

static const char *      __proc_meminfo_filename = "/proc/loadavg";
static struct proc_file *__pf_meminfo            = NULL;

int32_t init_collector_proc_meminfo() {
    return 0;
}

int32_t collector_proc_meminfo(int32_t update_every, usec_t dt, const char *config_path) {
    debug("[PLUGIN_PROC:proc_meminfo] config:%s running", config_path);

    const char *f_meminfo =
        appconfig_get_member_str(config_path, "monitor_file", __proc_meminfo_filename);

    if (unlikely(!__pf_meminfo)) {
        __pf_meminfo = procfile_open(f_meminfo, " \t:", PROCFILE_FLAG_DEFAULT);
        if (unlikely(!__pf_meminfo)) {
            error("Cannot open %s", f_meminfo);
            return -1;
        }
    }

    __pf_meminfo = procfile_readall(__pf_meminfo);
    if (unlikely(!__pf_meminfo)) {
        error("Cannot read %s", f_meminfo);
        return -1;
    }

    // 单位KB
    static uint64_t
        // 所有可用的内存大小，物理内存减去预留位和内核使用。系统从加电开始到引导完成，firmware/BIOS要预留一些内存，内核本身要占用一些内存，
        // 最后剩下可供内核支配的内存就是MemTotal。这个值在系统运行期间一般是固定不变的，重启会改变
        mem_total = 0,
        // 表示系统尚未使用的内存
        mem_free = 0,
        // 真正的系统可用内存，系统中有些内存虽然已被使用但是可以回收的，比如cache/buffer、slab都有一部分可以回收，所以这部分可回收的内存加上MemFree才是系统可用的内存
        mem_available = 0,
        // 用来给块设备做缓存的内存，(文件系统的 metadata、pages)
        buffers = 0,
        // 分配给文件缓冲区的内存,例如vi一个文件，就会将未保存的内容写到该缓冲区
        cached = 0,
        // 被高速缓冲存储用的交换空间（硬盘的swap）的大小
        swap_cached = 0,
        // 经常使用的高速缓冲存储器页面文件大小
        active = 0,
        // 不经常使用的高速缓冲存储器文件大小
        inactive = 0,
        // 活跃的匿名内存
        active_anon = 0,
        // 不活跃的匿名内存
        inactive_anon = 0,
        // 活跃的文件使用内存
        active_file = 0,
        // 不活跃的文件使用内存
        inactive_file = 0,
        // 不能被释放的内存页
        unevictable = 0,
        // 系统调用 mlock 家族允许程序在物理内存上锁住它的部分或全部地址空间。这将阻止Linux
        // 将这个内存页调度到交换空间（swap space），即使该程序已有一段时间没有访问这段空间
        mlocked = 0,
        // 交换空间总内存
        swap_total = 0,
        // 交换空间空闲内存
        swap_free = 0,
        // 等待被写回到磁盘的
        dirty = 0,
        // 正在被写回的
        writeback = 0,
        // 未映射页的内存/映射到用户空间的非文件页表大小
        anon_pages = 0,
        // 映射文件内存
        mapped = 0,
        // 已经被分配的共享内存
        shmem = 0,
        // 内核数据结构缓存
        slab = 0,
        // 可收回slab内存
        slab_reclaimable = 0,
        // 不可收回slab内存
        slab_unreclaimable = 0,
        // 内核消耗的内存
        kernel_stack = 0,
        // 管理内存分页的索引表的大小
        page_tables = 0,
        // 不稳定页表的大小
        nfs_unstable = 0,
        // 在低端内存中分配一个临时buffer作为跳转，把位于高端内存的缓存数据复制到此处消耗的内存
        bounce = 0,
        // FUSE用于临时写回缓冲区的内存
        writeback_tmp = 0,
        // 系统实际可分配内存
        commit_limit = 0,
        // 系统当前已分配的内存
        commited_as = 0,
        // 预留的虚拟内存总量
        vmalloc_total = 0,
        // 已经被使用的虚拟内存
        vmalloc_used = 0,
        // 可分配的最大的逻辑连续的虚拟内存
        vmalloc_chunk = 0,
        //
        percpu = 0,
        // 当系统检测到内存的硬件故障时删除掉的内存页的总量
        hardware_corrputed = 0,
        // 匿名大页缓存
        anon_huge_pages = 0,
        //
        shmem_huge_pages = 0,
        //
        shmem_pmdmapped = 0,
        //
        file_huge_pages = 0,
        //
        file_pmdmapped = 0,
        // 预留的大页内存总量
        huge_pages_total = 0,
        // 空闲的大页内存
        huge_pages_free = 0,
        // 已经被应用程序分配但尚未使用的大页内存
        huge_pages_rsvd = 0,
        // 初始大页数与修改配置后大页数的差值
        huge_pages_surp = 0,
        // 单个大页内存的大小
        huge_pages_size = 0,
        // 映射TLB为4kB的内存数量
        direct_map_4k = 0,
        // 映射TLB为2M的内存数量
        direct_map_2M = 0,
        // 映射TLB为1G的内存数量
        direct_map_1G = 0;

    size_t lines = procfile_lines(__pf_meminfo);

    return 0;
}

void fini_collector_proc_meminfo() {
}