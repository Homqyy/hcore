/**
 * @file hcore_list.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief 提供一个链表结构，且该结构提供了排序功能
 * @version 0.1
 * @date 2021-09-26
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#ifndef _HCORE_LIST_H_INCLUDED_
#define _HCORE_LIST_H_INCLUDED_

#include <hcore_constant.h>
#include <hcore_pool.h>

typedef struct hcore_list_s hcore_list_t;
typedef int (*hcore_list_compare_pt)(const void *p1, const void *p2);

// List
struct hcore_list_s
{
    hcore_pool_t *pool;

    unsigned int num_item;
    unsigned int num_reserved;

    void **p;

    hcore_list_compare_pt cmp;

    hcore_uint_t sorted : 1;
};

// Macro
#define HCORE_LIST_DATA(list, i) (((list) != NULL) ? ((list)->p[(i)]) : NULL)
#define HCORE_LIST_NUM(list)     (((list) != NULL) ? (list)->num_item : 0)

#define HCORE_LIST_INIT_NUM_RESERVED 32

/**
 * @brief  初始化链表结构
 * @note
 * @param  pool: 内存池
 * @param  list: 待初始化的链表结构
 * @param  compare: 链表的比较函数，其影响排序顺序
 * @retval
 * 初始化成功：HCORE_OK
 * 初始化失败：HCORE_ERROR
 */
hcore_int_t hcore_list_init(hcore_pool_t *pool, hcore_list_t *list,
                        hcore_list_compare_pt compare);

/**
 * @brief  创建链表结构
 * @note
 * @param  *pool: 内存池
 * @param  compare: 链表的比较函数，其影响排序顺序
 * @retval
 * 创建成功：链表结构
 * 创建失败：NULL
 */
hcore_list_t *hcore_list_create(hcore_pool_t *pool, hcore_list_compare_pt compare);

/**
 * @brief  添加节点'p'到链表中，此动作不会触发排序
 * @note
 * @param  *list: 链表结构，由'hcore_list_create()'创建或'hcore_list_init()'初始化
 * @param  *p: 待添加的节点
 * @retval
 */
hcore_int_t hcore_list_add(hcore_list_t *list, void *p);

/**
 * @brief  排序链表
 * @note
 * @param  *list: 待排序的链表结构
 * @retval None
 */
void hcore_list_sort(hcore_list_t *list);

/**
 * @brief  插入节点'p'到链表中，此动作会触发排序
 * @note
 * @param  *list: 链表结构
 * @param  *p: 待插入的节点
 * @retval
 */
hcore_int_t hcore_list_insert(hcore_list_t *list, void *p);

/**
 * @brief  从链表中删除节点'p'
 * @note
 * @param  *list: 链表结构
 * @param  *p: 待删除的节点
 * @retval
 */
hcore_int_t hcore_list_delete(hcore_list_t *list, void *p);

/**
 * @brief  清空链表
 * @note
 * @param  *list:
 * @retval None
 */
void hcore_list_delete_all(hcore_list_t *list);

/**
 * @brief  搜索节点 'target'
 * @note
 * @param  *list: 链表结构
 * @param  *target: 搜索的目标，有搜索所需的属性
 * @retval
 */
void *hcore_list_search(hcore_list_t *list, void *target);

#endif // !_HCORE_LIST_H_INCLUDED_
