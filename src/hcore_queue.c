/**
 * @file hcore_queue.c
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

#include <hcore_queue.h>

/*
 * find the middle queue element if the queue has odd number of elements
 * or the first element of the queue's second part otherwise
 */
hcore_queue_t *
hcore_queue_middle(hcore_queue_t *queue)
{
    hcore_queue_t *middle, *next;

    middle = hcore_queue_head(queue);

    if (middle == hcore_queue_last(queue)) { return middle; }

    next = hcore_queue_head(queue);

    for (;;)
    {
        middle = hcore_queue_next(middle);

        next = hcore_queue_next(next);

        if (next == hcore_queue_last(queue)) { return middle; }

        next = hcore_queue_next(next);

        if (next == hcore_queue_last(queue)) { return middle; }
    }
}

/* the stable insertion sort */
void
hcore_queue_sort(hcore_queue_t *queue,
               int (*cmp)(const hcore_queue_t *, const hcore_queue_t *))
{
    hcore_queue_t *q, *prev, *next;

    q = hcore_queue_head(queue);

    if (q == hcore_queue_last(queue)) { return; }

    for (q = hcore_queue_next(q); q != hcore_queue_sentinel(queue); q = next)
    {
        prev = hcore_queue_prev(q);
        next = hcore_queue_next(q);

        hcore_queue_remove(q);

        do {
            if (cmp(prev, q) <= 0) { break; }

            prev = hcore_queue_prev(prev);

        } while (prev != hcore_queue_sentinel(queue));

        hcore_queue_insert_after(prev, q);
    }
}