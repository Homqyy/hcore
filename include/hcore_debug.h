/**
 * @file hcore_debug.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief 提供调试接口
 * @version 0.1
 * @date 2021-09-26
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 * * mlist => memory list
 * * mnode => memory node
 */

#ifndef _HCORE_DEBUG_H_INCLUDED_
#define _HCORE_DEBUG_H_INCLUDED_

#include <assert.h>
#include <hcore_constant.h>
#include <hcore_list.h>

#define HCORE_DEBUG_MAX_STACK_NUM 64
#define HCORE_DEBUG_MAGIC_NUM     ((void *)0x01030507090B0D0F)

#ifdef _HCORE_DEBUG

#define HCORE_DEBUG_ASSERT_MNODE(node) \
    hcore_assert((node)->magic == HCORE_DEBUG_MAGIC_NUM)

typedef struct
{
    hcore_pool_t *pool;
    hcore_log_t  *log; // empty log
    hcore_uint_t  max_stack_num;
    hcore_list_t *mlist; // memory list

    hcore_uint64_t alloced_num;
    hcore_uint64_t free_num;

    hcore_uint64_t total_alloced_size;
    hcore_uint64_t total_free_size;
} hcore_debug_t;

typedef struct
{
    const char  *name; // last take '\0'
    size_t       size;
    hcore_uint_t bt_num;
    void        *magic;
    char       **bt_symbals;
    void        *addr; // address of memory

    hcore_uint_t debug : 1; // whether is node of debug
} hcore_debug_mnode_t;

/**
 * @brief  诊断表达式 'exp'，如果结果为假则 abort
 * @note
 * @retval
 */
#define hcore_assert(exp) assert(exp);

/**
 * @brief  如果被调用则直接 abort。
 * @note
 * @retval
 */
#define hcore_bug_on() assert(0);

/**
 * @brief  initialize debug environment
 * @note   once call for each process
 * @param  *log: log
 * @retval None
 */
void hcore_init_debug(hcore_log_t *log);

/**
 * @brief  deinit debug environment
 * @note   once call for each process
 * @retval None
 */
void hcore_deinit_debug(void);

hcore_debug_t *hcore_create_debug(hcore_log_t *log, hcore_uint_t max_stack_num);
void           hcore_destroy_debug(hcore_debug_t *db);
hcore_debug_t *hcore_get_debug(void);
void           hcore_set_debug(hcore_debug_t *db);

hcore_uchar_t *hcore_debug_get_memory_leak_info(hcore_uchar_t *buf, size_t len);

/**
 * @brief dump memory leak information to log
 *
 * @param log: log
 *
 * @retval None
 */
void hcore_debug_dump_memory_leak_info(hcore_log_t *log);

/**
 * @brief  writing leak information to log
 * @note
 * @retval None
 */
void hcore_debug_log_memory_leak_info(void);

/**
 * @brief create memory node for debug, then can check memory leak information
 * by 'hcore_debug_log_memory_leak_info' or 'hcore_debug_dump_memory_leak_info'
 * or 'hcore_debug_get_memory_leak_info'.
 *
 * @note 1. if you want to use this function, you must call 'hcore_init_debug'
 *       2. the mnode->addr as memory address that can be load and store
 *
 * @param  *name: constant string
 * @param  *size: size memory
 *
 * @retval hcore_debug_mnode_t* : memory node
 */
hcore_debug_mnode_t *hcore_debug_create_mnode(const char *name, size_t size);

/**
 * @brief
 * @note
 * @param  *node: created node by 'hcore_debug_create_mnode'
 * @retval None
 */
void hcore_debug_destroy_mnode(hcore_debug_mnode_t *node);

/**
 * @brief
 * @note
 * @param  *addr: mnode->addr
 * @retval
 */
hcore_debug_mnode_t *hcore_debug_get_mnode_of_addr(void *addr);

#else

#define hcore_assert(exp) \
    while (0)             \
    {                     \
    }
#define hcore_bug_on() \
    while (0)          \
    {                  \
    }
#define hcore_init_debug(log) \
    while (0)                 \
    {                         \
    }
#define hcore_deinit_debug() \
    while (0)                \
    {                        \
    }
#define hcore_debug_log_memory_leak_info() \
    while (0)                              \
    {                                      \
    }
#define hcore_debug_dump_memory_leak_info(log) \
    while (0)                                  \
    {                                          \
    }

#endif // _HCORE_DEBUG

#endif // !_HCORE_DEBUG_H_INCLUDED_
