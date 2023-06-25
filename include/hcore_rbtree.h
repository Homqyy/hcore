/**
 * @file hcore_rbtree.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief 提供红黑树接口
 * @version 0.1
 * @date 2021-09-26
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#ifndef _HCORE_RBTREE_H_INCLUDED_
#define _HCORE_RBTREE_H_INCLUDED_

#include <hcore_types.h>

typedef hcore_uint_t hcore_rbtree_key_t;
typedef hcore_int_t  hcore_rbtree_key_int_t;

typedef struct hcore_rbtree_node_s hcore_rbtree_node_t;

struct hcore_rbtree_node_s
{
    hcore_rbtree_key_t   key;
    hcore_rbtree_node_t *left;
    hcore_rbtree_node_t *right;
    hcore_rbtree_node_t *parent;
    hcore_uchar_t        color;
    hcore_uchar_t        data;
};

typedef struct hcore_rbtree_s hcore_rbtree_t;

typedef void (*hcore_rbtree_insert_pt)(hcore_rbtree_node_t *root,
                                     hcore_rbtree_node_t *node,
                                     hcore_rbtree_node_t *sentinel);

struct hcore_rbtree_s
{
    hcore_rbtree_node_t *  root;
    hcore_rbtree_node_t *  sentinel;
    hcore_rbtree_insert_pt insert;
};

#define hcore_rbtree_init(tree, s, i) \
    hcore_rbtree_sentinel_init(s);    \
    (tree)->root     = s;           \
    (tree)->sentinel = s;           \
    (tree)->insert   = i

hcore_rbtree_node_t *hcore_rbtree_next(hcore_rbtree_t *tree, hcore_rbtree_node_t *node);
void hcore_rbtree_insert(hcore_rbtree_t *tree, hcore_rbtree_node_t *node);
void hcore_rbtree_delete(hcore_rbtree_t *tree, hcore_rbtree_node_t *node);
void hcore_rbtree_insert_value(hcore_rbtree_node_t *root, hcore_rbtree_node_t *node,
                             hcore_rbtree_node_t *sentinel);

#define hcore_rbt_red(node)          ((node)->color = 1)
#define hcore_rbt_black(node)        ((node)->color = 0)
#define hcore_rbt_is_red(node)       ((node)->color)
#define hcore_rbt_is_black(node)     (!hcore_rbt_is_red(node))
#define hcore_rbt_copy_color(n1, n2) (n1->color = n2->color)

/* a sentinel must be black */
#define hcore_rbtree_sentinel_init(node) hcore_rbt_black(node)

static inline hcore_rbtree_node_t *
hcore_rbtree_min(hcore_rbtree_node_t *node, hcore_rbtree_node_t *sentinel)
{
    while (node->left != sentinel) { node = node->left; }
    return node;
}

#endif // !_HCORE_RBTREE_H_INCLUDED_
