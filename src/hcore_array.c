/**
 * @file hcore_array.c
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

#include <hcore_array.h>
#include <hcore_string.h>

hcore_array_t *
hcore_array_create(hcore_pool_t *p, hcore_uint_t n, size_t size)
{
    hcore_array_t *a;

    a = hcore_palloc(p, sizeof(hcore_array_t));
    if (a == NULL)
    {
        return NULL;
    }

    if (hcore_array_init(a, p, n, size) != HCORE_OK)
    {
        return NULL;
    }

    return a;
}

void
hcore_array_destroy(hcore_array_t *a)
{
    hcore_pool_t *p;

    p = a->pool;

    if ((u_char *)a->elts + a->size * a->nalloc == p->d.last)
    {
        p->d.last -= a->size * a->nalloc;
    }

    if ((u_char *)a + sizeof(hcore_array_t) == p->d.last)
    {
        p->d.last = (u_char *)a;
    }
}

void *
hcore_array_push(hcore_array_t *a)
{
    void         *elt, *new_elt;
    size_t        size;
    hcore_pool_t *p;

    if (a->nelts == a->nalloc)
    {
        /* the array is full */

        size = a->size * a->nalloc;
        p    = a->pool;

        if ((u_char *)a->elts + size == p->d.last
            && p->d.last + a->size <= p->d.end)
        {
            /*
             * the array allocation is the last in the pool
             * and there is space for new allocation
             */

            p->d.last += a->size;
            a->nalloc++;
        }
        else
        {
            /* allocate a new array */

            new_elt = hcore_palloc(p, 2 * size);
            if (new_elt == NULL)
            {
                return NULL;
            }

            hcore_memcpy(new_elt, a->elts, size);
            a->elts = new_elt;
            a->nalloc *= 2;
        }
    }

    elt = (u_char *)a->elts + a->size * a->nelts;
    a->nelts++;

    return elt;
}

void *
hcore_array_push_n(hcore_array_t *a, hcore_uint_t n)
{
    void         *elt, *new_elt;
    size_t        size;
    hcore_uint_t  nalloc;
    hcore_pool_t *p;

    size = n * a->size;

    if (a->nelts + n > a->nalloc)
    {
        /* the array is full */

        p = a->pool;

        if ((u_char *)a->elts + a->size * a->nalloc == p->d.last
            && p->d.last + size <= p->d.end)
        {
            /*
             * the array allocation is the last in the pool
             * and there is space for new allocation
             */

            p->d.last += size;
            a->nalloc += n;
        }
        else
        {
            /* allocate a new array */

            nalloc = 2 * ((n >= a->nalloc) ? n : a->nalloc);

            new_elt = hcore_palloc(p, nalloc * a->size);
            if (new_elt == NULL)
            {
                return NULL;
            }

            hcore_memcpy(new_elt, a->elts, a->nelts * a->size);
            a->elts   = new_elt;
            a->nalloc = nalloc;
        }
    }

    elt = (u_char *)a->elts + a->size * a->nelts;
    a->nelts += n;

    return elt;
}