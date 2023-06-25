/**
 * @file hcore_queue.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief
 * @version 0.1
 * @date 2021-11-19
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#ifndef _HCORE_QUEUE_H_INCLUDE_
#define _HCORE_QUEUE_H_INCLUDE_

#include <stdio.h>

typedef struct hcore_queue_s
{
    struct hcore_queue_s *prev;
    struct hcore_queue_s *next;
} hcore_queue_t;

#define hcore_queue_init(q) \
    (q)->prev = q;        \
    (q)->next = q

#define hcore_queue_empty(h) (h == (h)->prev)

#define hcore_queue_insert_head(h, x) \
    (x)->next       = (h)->next;    \
    (x)->next->prev = x;            \
    (x)->prev       = h;            \
    (h)->next       = x

#define hcore_queue_insert_after hcore_queue_insert_head

#define hcore_queue_insert_tail(h, x) \
    (x)->prev       = (h)->prev;    \
    (x)->prev->next = x;            \
    (x)->next       = h;            \
    (h)->prev       = x

#define hcore_queue_head(h) (h)->next

#define hcore_queue_last(h) (h)->prev

#define hcore_queue_sentinel(h) (h)

#define hcore_queue_next(q) (q)->next

#define hcore_queue_prev(q) (q)->prev

#if (_HCORE_DEBUG)

#define hcore_queue_remove(x)      \
    (x)->next->prev = (x)->prev; \
    (x)->prev->next = (x)->next; \
    (x)->prev       = NULL;      \
    (x)->next       = NULL

#else

#define hcore_queue_remove(x)      \
    (x)->next->prev = (x)->prev; \
    (x)->prev->next = (x)->next

#endif

#define hcore_queue_split(h, q, n) \
    (n)->prev       = (h)->prev; \
    (n)->prev->next = n;         \
    (n)->next       = q;         \
    (h)->prev       = (q)->prev; \
    (h)->prev->next = h;         \
    (q)->prev       = n;

#define hcore_queue_add(h, n)      \
    (h)->prev->next = (n)->next; \
    (n)->next->prev = (h)->prev; \
    (h)->prev       = (n)->prev; \
    (h)->prev->next = h;

#define hcore_queue_add_tail(h, n) hcore_queue_add(h, n)

#define hcore_queue_add_head(h, n) \
    (h)->next->prev = (n)->prev; \
    (n)->prev->next = (h)->next; \
    (h)->next       = (n)->next; \
    (h)->next->prev = h;

#define hcore_queue_data(q, type, link) \
    (type *)((u_char *)q - offsetof(type, link))

hcore_queue_t *hcore_queue_middle(hcore_queue_t *queue);

void hcore_queue_sort(hcore_queue_t *queue,
                    int (*cmp)(const hcore_queue_t *, const hcore_queue_t *));

#endif // !_HCORE_QUEUE_H_INCLUDE_
