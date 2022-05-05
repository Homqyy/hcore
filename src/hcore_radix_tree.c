/**
 * @file hcore_radix_tree.c
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

#include <hcore_radix_tree.h>

#define HCORE_MEMPAGE          8192 //内存页的大小，1kb
#define HCORE_INIT_POOL_SIZE   8192 //初始内存池大小
#define HCORE_INIT_NODE_NUM    32   //初定2*32
#define HCORE_RADIX_NO_VALUE   (hcore_uint_t) - 1
#define HCORE_RADIX_TREE_HIGHT (sizeof(hcore_uint_t) * 8 / HCORE_BITS) //树的高度

#define HCORE_CHECK_BITS(key, pos)                                         \
    ((((unsigned int)(key)) << (sizeof(int) * 8 - (pos + 1) * HCORE_BITS)) \
     >> (sizeof(int) * 8                                                 \
         - HCORE_BITS)) //返回key中由pos指定的位的值，位数由BITS指定

/**
 * 内存池扩大函数
 * 为内存池申请一片空间
 * @param tree_ptr
 * @param num :新内存池的大小，单位：页，每页大小为1024
 * @param ma：声明申请动态内存的接口
 * @return
 */
static hcore_radix_pool_t *
hcore_radixtree_get_newpool(hcore_radix_tree_t *t, int num)
{
    if (t == NULL) return NULL;
    if (num == -1) num = 1;
    hcore_radix_pool_t *pool =
        (hcore_radix_pool_t *)t->radixtree_malloc(num * HCORE_MEMPAGE);
    if (pool == NULL) return NULL;
    pool->start         = (char *)pool + sizeof(hcore_radix_pool_t);
    pool->size          = num * HCORE_MEMPAGE - sizeof(hcore_radix_pool_t);
    pool->next          = t->pool->next;
    pool->prev          = t->pool;
    t->pool->next->prev = pool;
    t->pool->next       = pool;
    t->pool             = pool;

    return pool;
}

/**
 * 节点创建函数
 * 创建一个节点，从内存池中取出可以使用的节点
 * @param tree_ptr
 * @param ma：声明申请动态内存的接口
 * @return
 */
static hcore_radix_node_t *
hcore_radixtree_node_alloc(hcore_radix_tree_t *t)
{
    int               i = 0;
    hcore_radix_node_t *node;

    if (t == NULL) return NULL;

    if (t->free != NULL) //从free中提取节点
    {
        node    = t->free;
        t->free = node->parent;
    }
    else //在内存池中寻找可以使用的内存
    {
        if (t->pool->size
            < sizeof(hcore_radix_node_t)) //如果剩余空间不够分配，则重新分配
        {
            hcore_radixtree_get_newpool(t, -1);
        }
        node = (hcore_radix_node_t *)t->pool->start;
        t->pool->start += sizeof(hcore_radix_node_t);
        t->pool->size -= sizeof(hcore_radix_node_t);
    }
    for (i = 0; i < HCORE_CHILD_NUM; i++) node->child[i] = NULL;

    node->parent = NULL;
    node->value  = NULL;

    return node;
}

/**
 * 管理树创建函数
 * @param ma：声明申请动态内存的接口
 * @return
 */
hcore_radix_tree_t *
hcore_radixtree_create(hcore_radixtree_malloc_pt ma, hcore_radixtree_free_pt fr)
{
    hcore_uint_t i = 0;
    if (NULL == ma) { ma = malloc; }
    hcore_radix_tree_t *tree = (hcore_radix_tree_t *)ma(sizeof(hcore_radix_tree_t));
    if (tree == NULL) return NULL;

    tree->radixtree_malloc = ma;
    tree->radixtree_free   = fr;
    char *p                = (char *)tree->radixtree_malloc(
        HCORE_INIT_POOL_SIZE); //为内存池结构分配空间

    hcore_radix_node_t *ns;
    if (!p)
    {
        tree->radixtree_free(tree);
        return NULL;
    }

    ((hcore_radix_pool_t *)p)->next = (hcore_radix_pool_t *)p;
    ((hcore_radix_pool_t *)p)->prev = (hcore_radix_pool_t *)p;

    ns = (hcore_radix_node_t *)((char *)p + sizeof(hcore_radix_pool_t));
    for (i = 1; i < HCORE_INIT_NODE_NUM - 1; i++) { ns[i].parent = &ns[i + 1]; }

    ns[i].parent = NULL;

    for (i = 0; i < HCORE_CHILD_NUM; i++) { ns[0].child[i] = NULL; }
    ns[0].parent = NULL;
    ns[0].value  = NULL;

    tree->pool = (hcore_radix_pool_t *)p;
    tree->root = ns;
    tree->free = &ns[1];
    ((hcore_radix_pool_t *)p)->start =
        (char *)ns + sizeof(hcore_radix_node_t) * HCORE_INIT_NODE_NUM;
    ((hcore_radix_pool_t *)p)->size =
        HCORE_INIT_POOL_SIZE - sizeof(hcore_radix_pool_t)
        - sizeof(hcore_radix_node_t) * HCORE_INIT_NODE_NUM;
    return tree;
}

/**
 * 节点插入函数
 * 插入节点时，每个节点的内存空间通过内存池分配
 * @param tree_ptr
 * @param key
 * @param mask
 * @param value
 * @return
 */
int
hcore_radixtree_insert(hcore_radix_tree_t *t, hcore_uint_t key, hcore_uint_t mask,
                     void *value, hcore_uint_t size)
{
    hcore_uint_t i, child_pos;

    hcore_radix_node_t *node, *child;

    if (t == NULL) { return HCORE_RADIX_ERROR; }
    node = t->root;
    for (i = 0; i < HCORE_RADIX_TREE_HIGHT; i++)
    {
        child_pos = HCORE_CHECK_BITS((key & mask), i);
        if (!node->child[child_pos])
        {
            child = hcore_radixtree_node_alloc(t);
            if (!child) { return HCORE_RADIX_ERROR; }
            child->parent          = node;
            node->child[child_pos] = child;
            node                   = node->child[child_pos];
        }
        else
        {
            node = node->child[child_pos];
        }
    }
    node->value = t->radixtree_malloc(size);
    if (node->value == NULL) return HCORE_RADIX_ERROR;
    memcpy(node->value, value, size);

    return HCORE_RADIX_OK;
}

/**
 * 节点删除函数
 * 由于插入时会创建很多节点，为了提高删除速度这里只会删除最底层的指定节点
 * @param tree_ptr
 * @param key
 * @param mask
 * @return
 */
int
hcore_radixtree_delete(hcore_radix_tree_t *t, hcore_uint_t key, hcore_uint_t mask)
{
    hcore_radix_node_t *node = t->root;
    hcore_uint_t        i = 0, child_pos = 0;

    if (node == NULL || t == NULL) return HCORE_RADIX_ERROR;
    while (
        node
        && i < HCORE_RADIX_TREE_HIGHT) // node为储存value的节点，在父节点中将此节点的链接置空，
    {
        child_pos = HCORE_CHECK_BITS((key & mask), i++);
        node      = node->child[child_pos];
    }
    //然后清空value后将此节点加入free中
    if (node == NULL) return HCORE_RADIX_ERROR;

    t->radixtree_free(node->value);
    node->value = NULL;
    // broadcast_mp_printf("value deleted\r\n");
    for (i = 0; i < HCORE_CHILD_NUM; i++)
    {
        if (node->parent->child[i] == node) { node->parent->child[i] = NULL; }
    }
    // node->parent->child[child_pos] = NULL;
    node->parent = t->free;
    t->free      = node;
    //仅回收叶子节点
    return HCORE_RADIX_OK;
}

/**
 * 节点查找函数
 * key为索引，返回叶节点被查找到的值
 * @param tree_ptr
 * @param key
 * @param mas
 * @return
 */
void *
hcore_radixtree_find(const hcore_radix_tree_t *t, hcore_uint_t key)
{
    hcore_uint_t        i    = 0, child_pos;
    hcore_radix_node_t *node = NULL;
    if (t == NULL) return NULL;
    node = t->root;
    while (node && i < HCORE_RADIX_TREE_HIGHT)
    {
        child_pos = HCORE_CHECK_BITS(key, i++);
        node      = node->child[child_pos];
    }
    if (node == NULL) return NULL;
    return node->value;
}

void
hcore_destroy_node(hcore_radix_tree_t *t, hcore_radix_node_t *node)
{
    int i;
    for (i = 0; i < HCORE_CHILD_NUM; i++)
    {
        if (node->child[i] != NULL) { hcore_destroy_node(t, node->child[i]); }
        else
        {
            if (node->value != NULL)
            {
                t->radixtree_free(node->value);
                node->value = NULL;
            }
        }
    }
}

/**
 * 销毁树
 * @param tree_ptr
 * @return
 */
void
hcore_radixtree_destroy(hcore_radix_tree_t *t)
{
    hcore_radix_pool_t *temp = NULL, *pre = t->pool->next;

    if (t == NULL) return;
    hcore_destroy_node(t, t->root);
    while (pre != t->pool)
    {
        temp = pre;
        pre  = pre->next;
        t->radixtree_free(temp);
    }
    t->radixtree_free(t->pool);
    t->radixtree_free(t);
}