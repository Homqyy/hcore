/**
 * @file hcore_buf.c
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

#include <hcore_buf.h>
#include <hcore_pool.h>

hcore_chain_t *
hcore_alloc_chain(hcore_pool_t *pool)
{
    hcore_chain_t *cl;

    cl = pool->chain;

    if (cl)
    {
        pool->chain = cl->next;

        cl->next = NULL;
        cl->free = NULL;
        return cl;
    }

    cl = hcore_pcalloc(pool, sizeof(hcore_chain_t));
    if (cl == NULL) { return NULL; }

    return cl;
}

void
hcore_free_chain(hcore_pool_t *pool, hcore_chain_t *cl)
{
    hcore_free_chain_hold_buf(pool, cl);
    cl->buf = NULL;
}

hcore_chain_t *
hcore_alloc_chain_with_buf(hcore_pool_t *pool)
{
    hcore_chain_t *cl;

    cl = hcore_alloc_chain(pool);
    if (cl == NULL) return NULL;

    if (cl->buf) return cl;

    cl->buf = hcore_pcalloc(pool, sizeof(hcore_buf_t));
    if (cl->buf == NULL) { return NULL; }

    /*
     * cl was cleared by hcore_pcalloc:
     *
     * cl->next = NULL;
     * cl->free = NULL;
     */

    return cl;
}

void
hcore_free_chain_hold_buf(hcore_pool_t *pool, hcore_chain_t *cl)
{
    if (cl->free) cl->free(cl);

    cl->free    = NULL;
    cl->next    = pool->chain;
    pool->chain = cl;
}

hcore_buf_t *
hcore_alloc_buf(hcore_pool_t *pool, size_t size)
{
    hcore_buf_t *b;

    b = hcore_pcalloc(pool, sizeof(hcore_buf_t));
    if (b == NULL) return NULL;

    b->last = b->pos = b->start = hcore_palloc(pool, size);
    if (b->start == NULL) return NULL;

    b->end = b->start + size;

    return b;
}