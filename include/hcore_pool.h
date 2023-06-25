/**
 * @file hcore_pool.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief 提供内存池，通过内存池可以方便的管理内存
 * @version 0.1
 * @date 2021-09-26
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#ifndef _HCORE_POOL_H_INCLUDED_
#define _HCORE_POOL_H_INCLUDED_


#include <hcore_log.h>
#include <hcore_types.h>

#define HCORE_POOL_SIZE_DEFAULT (16 * 1024)

typedef struct hcore_pool_large_s   hcore_pool_large_t;
typedef struct hcore_pool_data_s    hcore_pool_data_t;
typedef struct hcore_pool_cleanup_s hcore_pool_cleanup_t;
typedef void (*hcore_pool_clean_handler_pt)(void *data);
typedef void *(*hcore_pool_alloc_pt)(void *pool, size_t size);
typedef void (*hcore_pool_free_pt)(void *pool, void *p);
typedef void (*hcore_pool_destroy_pt)(void *pool);

struct hcore_pool_cleanup_s
{
    hcore_pool_clean_handler_pt handler;
    void                       *data;
    hcore_pool_cleanup_t       *next;
};

struct hcore_pool_large_s
{
    struct hcore_pool_large_s *next;
    void                      *alloc;
};

struct hcore_pool_data_s
{
    u_char       *last;
    u_char       *end;
    hcore_pool_t *next;
    hcore_uint_t  failed;
};

struct hcore_custom_pool_s
{
    void                 *pool;
    hcore_pool_alloc_pt   alloc;
    hcore_pool_free_pt    free;
    hcore_pool_destroy_pt destroy;
};

struct hcore_pool_s
{
    hcore_custom_pool_t custom;

    hcore_pool_data_t     d;
    size_t                max;
    hcore_pool_t         *current;
    hcore_pool_large_t   *large;
    hcore_log_t          *log;
    hcore_chain_t        *chain;
    hcore_pool_cleanup_t *cleanup;

    hcore_uint_t customed : 1;
};

hcore_pool_t *hcore_create_custom_pool(hcore_log_t *log, void *pool,
                                       hcore_pool_alloc_pt   alloc,
                                       hcore_pool_free_pt    free,
                                       hcore_pool_destroy_pt destroy);

/**
 * @brief  创建内存池
 * @note
 * @param  size:
 * 单池的大小；内存池实际上是多个单池以链表的形式串接起来的，因此单词申请的大小不能超过此'size'
 * @param  *log: 日志
 * @retval
 */
hcore_pool_t *hcore_create_pool(size_t size, hcore_log_t *log);

/**
 * @brief  销毁内存池
 * @note
 * @param  *pool:
 * @retval None
 */
void hcore_destroy_pool(hcore_pool_t *pool);

/**
 * @brief  释放大块内存'p'；
 * @note   由于释放一块内存池的内存是非常消耗性能的，
 * * 因此只有大内存才有必要调用此接口去释放。
 * @param  *pool:
 * @param  *p:
 * @retval
 */
hcore_int_t hcore_pfree(hcore_pool_t *pool, void *p);

/**
 * @brief  申请一块安'alignment'排列的内存，空间大小为'size'
 * @note
 * @param  alignment: 排列的基数
 * @param  size: 申请的空间大小
 * @param  *log:
 * @retval
 * 成功：分配的空间地址
 * 失败：NULL
 */
void *hcore_memalign(size_t alignment, size_t size, hcore_log_t *log);

/**
 * @brief  pool no align alloc; 申请一块'size'大小的空间，地址不进行排列
 * @note
 * @param  *pool:
 * @param  size:
 * @retval
 * 成功：分配的空间地址
 * 失败：NULL
 */
void *hcore_pnalloc(hcore_pool_t *pool, size_t size);

/**
 * @brief  pool clear alloc; 申请一块'size'大小的空间，并将空间的值都设置为0
 * @note
 * @param  *pool:
 * @param  size:
 * @retval
 * 成功：分配的空间地址
 * 失败：NULL
 */
void *hcore_pcalloc(hcore_pool_t *pool, size_t size);

/**
 * @brief  重新分配'p'的空间大小
 * @note
 * @param  *pool: 内存池
 * @param  *p: 待重新分配空间的'p'
 * @param  old_size: 原本空间的大小
 * @param  new_size: 新分配空间的大小
 * @retval
 * 成功：返回分配后的空间地址
 * 失败：NULL
 */
void *hcore_prealloc(hcore_pool_t *pool, void *p, size_t old_size,
                     size_t new_size);

/**
 * @brief  pool align alloc; 申请一块'size'大小的空间，地址进行排列
 * @note
 * @param  *pool:
 * @param  size:
 * @retval
 */
#define hcore_palloc(pool, size) hcore_pnalloc(pool, size)

hcore_chain_t *hcore_alloc_chain(hcore_pool_t *pool);
void           hcore_free_chain(hcore_pool_t *pool, hcore_chain_t *cl);
hcore_chain_t *hcore_alloc_chain_with_buf(hcore_pool_t *pool);
void           hcore_free_chain_hold_buf(hcore_pool_t *pool, hcore_chain_t *cl);
hcore_buf_t   *hcore_alloc_buf(hcore_pool_t *pool, size_t size);
hcore_pool_cleanup_t *hcore_pool_cleanup_add(hcore_pool_t *pool,
                                             size_t        data_size);

#endif // !_HCORE_POOL_H_INCLUDED_
