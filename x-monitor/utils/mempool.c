/*
 * @Author: CALM.WU
 * @Date: 2022-01-24 14:47:41
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2022-01-24 17:32:56
 */

#include "compiler.h"
#include "log.h"
#include "mempool.h"

#define XM_ALIGNMENT_SIZE sizeof(uint64_t)

struct xm_mempool_block_s {
    uint32_t block_size;              // The size of the memory block(), in bytes
    uint32_t free_unit_count;         // The number of free units in the block
    uint32_t free_unit_pos;           // The sequence number of the free unit, starting from 0
    struct xm_mempool_block_s *next;  // The next block
    char                       data[FLEX_ARRAY];  // The data
};

struct xm_mempool_s {
    pthread_spinlock_t lock;

    uint32_t                   unit_size;             // The size of the unit, in bytes
    int32_t                    init_mem_unit_count;   // The number of memory units to initialize
    int32_t                    grow_mem_unit_count;   // The number of memory units to grow
    int32_t                    curr_mem_block_count;  // The number of memory blocks currently
    struct xm_mempool_block_s *root;                  // The root block
};

static struct xm_mempool_block_s *xm_memblock_create(uint32_t unit_size, int32_t unit_count) {
    struct xm_mempool_block_s *block = NULL;

    block = (struct xm_mempool_block_s *)malloc(sizeof(struct xm_mempool_block_s)
                                                + unit_size * unit_count);

    if (unlikely(NULL == block)) {
        error("malloc xm_mempool_block_s failed");
        return NULL;
    }

    block->next            = NULL;
    block->free_unit_pos   = 0;
    block->block_size      = unit_size * unit_count;
    block->free_unit_count = unit_count;

    char *offset = block->data;
    for (int32_t i = 1; i < unit_count; i++) {
        // example, unit[0].next_freepos = 1
        *((int32_t *)offset) = i;
        offset += unit_size;
    }
    return block;
}

struct xm_mempool_s *xm_mempool_init(uint32_t unit_size, int32_t init_mem_unit_count,
                                     int32_t grow_mem_unit_count) {
    struct xm_mempool_s *pool = NULL;

    if (unlikely(init_mem_unit_count <= 0 || grow_mem_unit_count <= 0)) {
        error("invalid init_mem_unit_count(%d) or grow_mem_unit_count(%d)", init_mem_unit_count,
              grow_mem_unit_count);
        return NULL;
    }

    pool = (struct xm_mempool_s *)calloc(1, sizeof(struct xm_mempool_s));
    if (unlikely(NULL == pool)) {
        error("calloc failed");
        return NULL;
    }

    // unit_size必须大于sizeof(int32_t)，这里存放下一个free unit的下标
    // 这个空间是复用的，空闲时记录空闲下标，分配时全部可用
    if (unlikely(unit_size < sizeof(int32_t))) {
        pool->unit_size = sizeof(int32_t);
    }

    /* round up to a 'XM_ALIGNMENT_SIZE' alignment */
    pool->unit_size            = ROUNDUP(unit_size, XM_ALIGNMENT_SIZE);
    pool->curr_mem_block_count = 0;
    pool->init_mem_unit_count  = init_mem_unit_count;
    pool->grow_mem_unit_count  = grow_mem_unit_count;
    pool->root                 = NULL;
    pthread_spin_init(&pool->lock, PTHREAD_PROCESS_SHARED);

    debug("mempool init ok! unit_size: %d, init_mem_unit_count: %d, grow_mem_unit_count: %d",
          pool->unit_size, pool->init_mem_unit_count, pool->grow_mem_unit_count);

    return pool;
}

void xm_mempool_fini(struct xm_mempool_s *pool) {
    struct xm_mempool_block_s *block = NULL;
    struct xm_mempool_block_s *temp  = NULL;

    if (unlikely(NULL == pool || NULL == pool->root)) {
        return;
    }

    block = pool->root;
    while (block) {
        temp  = block;
        block = block->next;
        free(temp);
    }

    free(pool);
    pool = NULL;

    debug("mempool fini ok!");
}

void *xm_mempool_malloc(struct xm_mempool_s *pool) {
    return NULL;
}

int32_t xm_mempool_free(struct xm_mempool_s *pool, void *pfree) {
    return 0;
}

void print_mempool_info(struct xm_mempool_s *pool) {
}

void print_mempool_block_info_by_pointer(struct xm_mempool_s *pool, void *pblock) {
}