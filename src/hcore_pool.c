/**
 * @file hcore_pool.c
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

#include <hcore_base.h>
#include <hcore_debug.h>
#include <hcore_lib.h>
#include <hcore_pool.h>
#include <hcore_string.h>

#include <stdlib.h>

#define HCORE_POOL_ALIGNMENT 16

static inline void *hcore_palloc_small(hcore_pool_t *pool, size_t size,
                                     hcore_uint_t align);

static void *hcore_palloc_block(hcore_pool_t *pool, size_t size);
static void *hcore_palloc_large(hcore_pool_t *pool, size_t size);

void *
hcore_prealloc(hcore_pool_t *pool, void *p, size_t old_size, size_t new_size)
{
    void *      new_p;
    hcore_pool_t *node;

    if (p == NULL) { return hcore_palloc(pool, new_size); }

    if (new_size == 0)
    {
        if ((u_char *)p + old_size == pool->d.last) { pool->d.last = p; }
        else
        {
            hcore_pfree(pool, p);
        }

        return NULL;
    }

    if (old_size <= pool->max)
    {
        for (node = pool; node; node = node->d.next)
        {
            if ((u_char *)p + old_size == node->d.last
                && (u_char *)p + new_size <= node->d.end)
            {
                node->d.last = (u_char *)p + new_size;
                return p;
            }
        }
    }

    if (new_size <= old_size) { return p; }

    new_p = hcore_palloc(pool, new_size);
    if (new_p == NULL) { return NULL; }

    hcore_memcpy(new_p, p, old_size);

    hcore_pfree(pool, p);

    return new_p;
}

void *
hcore_memalign(size_t alignment, size_t size, hcore_log_t *log)
{
#ifdef _HCORE_DEBUG
    hcore_debug_mnode_t *node = hcore_debug_create_mnode("hcore_memalign", size);
    if (node == NULL) return NULL;

    return node->addr;
#else
    void *p;
    int   err;

    err = posix_memalign(&p, alignment, size);

    if (err)
    {
        hcore_log_error(HCORE_LOG_EMERG, log, err,
                      "posix_memalign(%uz, %uz) failed", alignment, size);
        p = NULL;
    }

    hcore_log_debug(log, 0, "posix_memalign: %p:%uz @%uz", p, size, alignment);

    return p;
#endif // _HCORE_DEBUG
}

void
hcore_destroy_pool(hcore_pool_t *pool)
{
    hcore_pool_t *        p, *n;
    hcore_pool_large_t *  l;
    hcore_pool_cleanup_t *cleanup;

    for (cleanup = pool->cleanup; cleanup; cleanup = cleanup->next)
    {
        if (cleanup->handler)
        {
            hcore_log_debug(pool->log, 0, "run cleanup: %p", cleanup);
            cleanup->handler(cleanup->data);
        }
    }

    for (l = pool->large; l; l = l->next)
    {
        if (l->alloc) { hcore_free(l->alloc); }
    }

    for (p = pool, n = pool->d.next; /* void */; p = n, n = n->d.next)
    {
        hcore_free(p);

        if (n == NULL) { break; }
    }
}

hcore_pool_t *
hcore_create_pool(size_t size, hcore_log_t *log)
{
    hcore_pool_t *p;
    int         hcore_pagesize;

    p = hcore_memalign(HCORE_POOL_ALIGNMENT, size, log);
    if (p == NULL) { return NULL; }

    p->d.last   = (hcore_uchar_t *)p + sizeof(hcore_pool_t);
    p->d.end    = (hcore_uchar_t *)p + size;
    p->d.next   = NULL;
    p->d.failed = 0;

    hcore_pagesize = hcore_getpagesize();

    size   = size - sizeof(hcore_pool_t);
    p->max = (size < hcore_pagesize - 1) ? size : hcore_pagesize - 1;

    p->current = p;
    p->chain   = NULL;
    p->large   = NULL;
    p->log     = log;
    p->cleanup = NULL;

    return p;
}

hcore_pool_cleanup_t *
hcore_pool_cleanup_add(hcore_pool_t *pool, size_t data_size)
{
    hcore_pool_cleanup_t *c;

    c = hcore_palloc(pool, sizeof(hcore_pool_cleanup_t));
    if (c == NULL) { return NULL; }

    if (data_size)
    {
        c->data = hcore_palloc(pool, data_size);
        if (c->data == NULL) { return NULL; }
    }
    else
    {
        c->data = NULL;
    }

    c->handler = NULL;
    c->next    = pool->cleanup;

    pool->cleanup = c;

    return c;
}

void *
hcore_pcalloc(hcore_pool_t *pool, size_t size)
{
    void *p;

    p = hcore_palloc(pool, size);
    if (p) { hcore_memzero(p, size); }

    return p;
}

void *
hcore_pnalloc(hcore_pool_t *pool, size_t size)
{
    if (size <= pool->max) { return hcore_palloc_small(pool, size, 0); }

    return hcore_palloc_large(pool, size);
}

static inline void *
hcore_palloc_small(hcore_pool_t *pool, size_t size, hcore_uint_t align)
{
    hcore_uchar_t *m;
    hcore_pool_t * p;

    p = pool->current;

    do {
        m = p->d.last;

        if (align) { m = hcore_align_ptr(m, HCORE_ALIGNMENT); }

        if ((size_t)(p->d.end - m) >= size)
        {
            p->d.last = m + size;

            return m;
        }

        p = p->d.next;

    } while (p);

    return hcore_palloc_block(pool, size);
}

static void *
hcore_palloc_block(hcore_pool_t *pool, size_t size)
{
    hcore_uchar_t *m;
    size_t       psize;
    hcore_pool_t * p, *new_pool;

    psize = (size_t)(pool->d.end - (hcore_uchar_t *)pool);

    m = hcore_memalign(HCORE_POOL_ALIGNMENT, psize, pool->log);
    if (m == NULL) { return NULL; }

    new_pool = (hcore_pool_t *)m;

    new_pool->d.end    = m + psize;
    new_pool->d.next   = NULL;
    new_pool->d.failed = 0;

    m += sizeof(hcore_pool_data_t);
    m                = hcore_align_ptr(m, HCORE_ALIGNMENT);
    new_pool->d.last = m + size;

    for (p = pool->current; p->d.next; p = p->d.next)
    {
        if (p->d.failed++ > 4) { pool->current = p->d.next; }
    }

    p->d.next = new_pool;

    return m;
}

static void *
hcore_palloc_large(hcore_pool_t *pool, size_t size)
{
    void *            p;
    hcore_uint_t        n;
    hcore_pool_large_t *large;

    p = hcore_malloc(size);
    if (p == NULL) { return NULL; }

    n = 0;

    for (large = pool->large; large; large = large->next)
    {
        if (large->alloc == NULL)
        {
            large->alloc = p;
            return p;
        }

        if (n++ > 3) { break; }
    }

    large = hcore_palloc_small(pool, sizeof(hcore_pool_large_t), 1);
    if (large == NULL)
    {
        hcore_free(p);
        return NULL;
    }

    large->alloc = p;
    large->next  = pool->large;
    pool->large  = large;

    return p;
}

hcore_int_t
hcore_pfree(hcore_pool_t *pool, void *p)
{
    hcore_pool_large_t *l;

    for (l = pool->large; l; l = l->next)
    {
        if (p == l->alloc)
        {
            hcore_log_debug(pool->log, 0, "free: %p", l->alloc);
            hcore_free(l->alloc);
            l->alloc = NULL;

            return HCORE_OK;
        }
    }

    return HCORE_ERROR;
}