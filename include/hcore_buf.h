/**
 * @file hcore_buf.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief
 * @version 0.1
 * @date 2021-09-29
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr: buf => buffer
 */

#ifndef _HCORE_BUF_H_INCLUDED_
#define _HCORE_BUF_H_INCLUDED_

#include <hcore_pool.h>
#include <hcore_types.h>

struct hcore_buf_s
{
    hcore_uchar_t *pos;
    hcore_uchar_t *last;

    hcore_uchar_t *start; /* start of buffer */
    hcore_uchar_t *end;   /* end of buffer */
};

struct hcore_chain_s
{
    hcore_buf_t *  buf;
    hcore_chain_t *next;
    void (*free)(struct hcore_chain_s *cl);
    void *data;
};

#define hcore_buf_get_freesize(b) ((b)->end - (b)->last)
#define hcore_buf_get_size(b)     ((b)->last - (b)->pos)

hcore_chain_t *hcore_alloc_chain(hcore_pool_t *pool);
void         hcore_free_chain(hcore_pool_t *pool, hcore_chain_t *cl);
hcore_chain_t *hcore_alloc_chain_with_buf(hcore_pool_t *pool);
void         hcore_free_chain_hold_buf(hcore_pool_t *pool, hcore_chain_t *cl);
hcore_buf_t *  hcore_alloc_buf(hcore_pool_t *pool, size_t size);

#endif // !_HCORE_BUF_H_INCLUDED_
