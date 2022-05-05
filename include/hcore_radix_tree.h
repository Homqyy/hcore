/**
 * @file hcore_radix_tree.h
 * @author yang_huajun (yang_huajun@topesc.com.cn)
 * @brief
 * @version 0.1
 * @date 2021-10-18
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#ifndef _HCORE_RADIX_TREE_H_
#define _HCORE_RADIX_TREE_H_

#include <hcore_types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HCORE_RADIX_OK    0  //操作成功
#define HCORE_RADIX_ERROR -1 //操作失败
#define HCORE_BITS        1
#define HCORE_CHILD_NUM   2 // HCORE_CHILD_NUM = 2^HCORE_BITS HCORE_BITS会改变基数树的层数

typedef void *(*hcore_radixtree_malloc_pt)(size_t size);
typedef void (*hcore_radixtree_free_pt)(void *ptr);

//基数树节点
typedef struct hcore_radix_node_s
{
    struct hcore_radix_node_s *child[HCORE_CHILD_NUM];
    struct hcore_radix_node_s *parent;
    void *                   value;
} hcore_radix_node_t;

//内存池描述结构，放在内存池的前段
typedef struct hcore_radix_pool_s
{
    struct hcore_radix_pool_s *next; //内存池是双向循环链表的一个节点
    struct hcore_radix_pool_s *prev;
    char *     start; //已分配内存中还未使用的内存首地址
    hcore_uint_t size;  //已分配内存中还未使用的内存长度
} hcore_radix_pool_t;

//基数树管理结构
typedef struct hcore_radix_tree_s
{
    hcore_radix_node_t *root; //根节点
    hcore_radix_pool_t *pool; //内存池指针
    hcore_radix_node_t
        *                   free; //储存已分配但不在树中的节点（双向链表，这里储存其中的一个节点）
    hcore_radixtree_malloc_pt radixtree_malloc;
    hcore_radixtree_free_pt   radixtree_free;
} hcore_radix_tree_t;


static hcore_radix_pool_t *hcore_radixtree_get_newpool(hcore_radix_tree_t *t,
                                                   int               num);

static hcore_radix_node_t *hcore_radixtree_node_alloc(hcore_radix_tree_t *t);

hcore_radix_tree_t *hcore_radixtree_create(hcore_radixtree_malloc_pt ma,
                                       hcore_radixtree_free_pt   fr);

int hcore_radixtree_insert(hcore_radix_tree_t *t, hcore_uint_t key, hcore_uint_t mask,
                         void *value, hcore_uint_t size);

int hcore_radixtree_delete(hcore_radix_tree_t *t, hcore_uint_t key, hcore_uint_t mask);
void *hcore_radixtree_find(const hcore_radix_tree_t *t, hcore_uint_t key);
void  hcore_destroy_node(hcore_radix_tree_t *t, hcore_radix_node_t *node);
void  hcore_radixtree_destroy(hcore_radix_tree_t *t);

#endif // !_HCORE_RADIX_TREE_H_
