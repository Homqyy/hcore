/**
 * @file hcore_debug.c
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief
 * @version 0.1
 * @date 2021-10-18
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#ifdef _HCORE_DEBUG

#include <hcore_base.h>
#include <hcore_debug.h>
#include <hcore_string.h>

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

static hcore_debug_t *g_hcore_debug;

static int hcore_debug_mnode_cmp(const void *p1, const void *p2);

hcore_uchar_t *
hcore_debug_get_memory_leak_info(hcore_uchar_t *buf, size_t len)
{
    hcore_debug_t *db = hcore_get_debug();
    hcore_uchar_t *p, *last;
    hcore_int_t    i, j;
    unsigned int   num;

    hcore_assert(db);

    if (db == NULL) return buf;

    p    = buf;
    last = buf + len;

    num = HCORE_LIST_NUM(db->mlist);

    if (db->alloced_num != db->free_num)
    {
        p = hcore_slprintf(p, last, "\nHave a memory leak!!!\n");
        p = hcore_slprintf(p, last, "Have a memory leak!!!\n");
        p = hcore_slprintf(p, last, "Have a memory leak!!!\n");
    }

    p = hcore_slprintf(p, last, "\n##### Memory Leak Information #####\n");
    p = hcore_slprintf(p, last, "#\n");

    p = hcore_slprintf(p, last, "# Leak Node Number       : %ud\n", num);

    p = hcore_slprintf(p, last, "# Leak Memory Size       : ");
    p = hcore_strlfmt_size(db->total_alloced_size - db->total_free_size, p,
                           last);
    p = hcore_slprintf(p, last, "\n");

    p = hcore_slprintf(p, last, "#\n");
    p = hcore_slprintf(p, last, "# Allocated Node Number  : %uL\n",
                       db->alloced_num);

    p = hcore_slprintf(p, last, "# Allocated Memory Size  : ");
    p = hcore_strlfmt_size(db->total_alloced_size, p, last);
    p = hcore_slprintf(p, last, "\n");

    p = hcore_slprintf(p, last, "# Free Node Number       : %uL\n",
                       db->free_num);
    p = hcore_slprintf(p, last, "# Free Memory Size       : ");
    p = hcore_strlfmt_size(db->total_free_size, p, last);
    p = hcore_slprintf(p, last, "\n");

    p = hcore_slprintf(p, last, "#\n");
    p = hcore_slprintf(p, last, "##################################\n");

    for (i = 0; i < num; i++)
    {
        hcore_debug_mnode_t *node = db->mlist->p[i];

        HCORE_DEBUG_ASSERT_MNODE(node);

        p = hcore_slprintf(p, last, "Name: %s\n", node->name);
        p = hcore_slprintf(p, last, "Size: %uz B\n", node->size);

        for (j = 0; j < node->bt_num; j++)
        {
            p = hcore_slprintf(p, last, "    [%i] %s\n", j,
                               node->bt_symbals[j]);
        }
    }

    return p;
}

void
hcore_debug_log_memory_leak_info(void)
{
    hcore_debug_t *db = hcore_get_debug();
    hcore_uchar_t  info[HCORE_LOG_ERRSTR_LENGTH_MAX];
    hcore_uchar_t *last;

    hcore_memzero(info, HCORE_LOG_ERRSTR_LENGTH_MAX);

    hcore_assert(db);

    if (db == NULL) return;

    last = hcore_debug_get_memory_leak_info(info, sizeof(info));

    if (last == info) return;

    hcore_log_debug(db->log, 0, "%*s", last - info, info);
}

hcore_debug_mnode_t *
hcore_debug_get_mnode_of_addr(void *addr)
{
    hcore_assert(addr);

    return (hcore_debug_mnode_t *)addr - 1;
}

void
hcore_debug_destroy_mnode(hcore_debug_mnode_t *node)
{
    hcore_debug_t *db = hcore_get_debug();

    hcore_assert(node);

    HCORE_DEBUG_ASSERT_MNODE(node);

    if (node->debug)
    {
        db->free_num++;
        db->total_free_size += node->size;

        if (node->bt_symbals) free(node->bt_symbals);

        hcore_list_delete(db->mlist, node);
    }

    free(node);
}

hcore_debug_mnode_t *
hcore_debug_create_mnode(const char *name, size_t size)
{
    hcore_debug_t       *db = hcore_get_debug();
    hcore_debug_mnode_t *node;
    void                *bt[HCORE_DEBUG_MAX_STACK_NUM] = {0};
    void               **pbt;
    int                  max_num;
    int                  num;

    hcore_assert(size);

    node = malloc(sizeof(hcore_debug_mnode_t)
                  + size); // don't use interface of hcore_xxx for malloc

    if (node == NULL) return NULL;

    memset(node, 0x00, sizeof(hcore_debug_mnode_t));

    node->magic = HCORE_DEBUG_MAGIC_NUM;
    node->name  = name;
    node->size  = size;

    if (db)
    {
        // backtrace
        pbt     = bt;
        max_num = hcore_min(db->max_stack_num, HCORE_DEBUG_MAX_STACK_NUM);

        num = backtrace(pbt, max_num);

        node->bt_num     = num;
        node->bt_symbals = backtrace_symbols(pbt, num);

        if (1 < node->bt_num)
        {
            pbt++;
            node->bt_num--;
        }
        else
        {
            hcore_bug_on();
        }

        if (hcore_list_insert(db->mlist, node) != HCORE_OK) goto error;

        node->debug = 1;

        // stat

        db->alloced_num++;
        db->total_alloced_size += node->size;
    }

    node->addr = node + 1;

    return node;

error:
    if (node)
    {
        if (node->bt_symbals) free(node->bt_symbals);
        free(node);
    }

    return NULL;
}

void
hcore_init_debug(hcore_log_t *log)
{
    hcore_debug_t *db = hcore_create_debug(log, 16);

    hcore_set_debug(db);
}

void
hcore_deinit_debug(void)
{
    hcore_debug_t *db = hcore_get_debug();
    hcore_destroy_debug(db);
}

hcore_debug_t *
hcore_create_debug(hcore_log_t *log, hcore_uint_t max_stack_num)
{
    hcore_debug_t *db;
    hcore_pool_t  *pool = NULL;
    hcore_log_t   *new_log;

    pool = hcore_create_pool(HCORE_POOL_SIZE_DEFAULT, log);
    if (pool == NULL) return NULL;

    new_log = hcore_pnalloc(pool, sizeof(hcore_log_t));
    if (new_log == NULL) return NULL;

    *new_log = *log;

    db = hcore_pnalloc(pool, sizeof(hcore_debug_t));
    if (db == NULL) goto error;

    hcore_memzero(db, sizeof(hcore_debug_t));
    db->log           = new_log;
    db->pool          = pool;
    db->max_stack_num = max_stack_num;

    db->mlist = hcore_list_create(NULL, hcore_debug_mnode_cmp);
    if (db->mlist == NULL) goto error;

    return db;

error:
    return NULL;
}

static int
hcore_debug_mnode_cmp(const void *p1, const void *p2)
{
    hcore_debug_mnode_t *n1 = *(hcore_debug_mnode_t **)p1;
    hcore_debug_mnode_t *n2 = *(hcore_debug_mnode_t **)p2;

    if (n1->name == NULL) return -1;

    if (n2->name == NULL) return 1;

    return strcasecmp(n1->name, n2->name);
}

void
hcore_destroy_debug(hcore_debug_t *db)
{
    hcore_int_t i;

    hcore_assert(db);

    for (i = 0; i < HCORE_LIST_NUM(db->mlist); i++)
    {
        hcore_debug_destroy_mnode(db->mlist->p[i]);
    }

    hcore_list_destroy(db->mlist);

    hcore_destroy_pool(db->pool);
}

void
hcore_set_debug(hcore_debug_t *db)
{
    g_hcore_debug = db;
}

hcore_debug_t *
hcore_get_debug(void)
{
    return g_hcore_debug;
}

#endif // !_HCORE_DEBUG
