/**
 * @file hcore_list.c
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

#include <hcore_list.h>

#include <stdlib.h>

static void *hcore_list_alloc(hcore_pool_t *pool, size_t size);
static void *hcore_list_realloc(hcore_pool_t *pool, void *p, size_t old_size,
                                size_t new_size);

hcore_int_t
hcore_init_list(hcore_pool_t *pool, hcore_list_t *list,
                hcore_list_compare_pt compare)
{
    list->num_reserved = HCORE_LIST_INIT_NUM_RESERVED;
    list->num_item     = 0;
    list->pool         = pool;

    list->p = hcore_list_alloc(list->pool, sizeof(void *) * list->num_reserved);
    if (list->pool == NULL)
    {
        return HCORE_ERROR;
    }

    list->cmp    = compare;
    list->sorted = 1;

    return HCORE_OK;
}

static inline void *
hcore_list_alloc(hcore_pool_t *pool, size_t size)
{
    if (pool)
    {
        return hcore_palloc(pool, size);
    }
    else
    {
        return malloc(size);
    }
}

static inline void *
hcore_list_realloc(hcore_pool_t *pool, void *p, size_t old_size,
                   size_t new_size)
{
    if (pool)
    {
        return hcore_prealloc(pool, p, old_size, new_size);
    }
    else
    {
        return realloc(p, new_size);
    }
}

hcore_list_t *
hcore_create_list(hcore_pool_t *pool, hcore_list_compare_pt compare)
{
    hcore_list_t *list;

    list = hcore_list_alloc(pool, sizeof(hcore_list_t));

    if (list == NULL)
    {
        return NULL;
    }

    if (hcore_init_list(pool, list, compare) != HCORE_OK)
    {
        return NULL;
    }

    return list;
}

hcore_int_t
hcore_list_add(hcore_list_t *list, void *p)
{
    unsigned int i;
    unsigned int old_num;


    if (list == NULL || p == NULL)
    {
        return HCORE_ERROR;
    }

    i = list->num_item;
    list->num_item++;

    if (list->num_item > list->num_reserved)
    {
        old_num            = list->num_reserved;
        list->num_reserved = list->num_reserved * 2;

        list->p =
            hcore_list_realloc(list->pool, list->p, old_num * sizeof(void *),
                               list->num_reserved * sizeof(void *));
        if (list->p == NULL)
        {
            return HCORE_ERROR;
        }
    }

    list->p[i] = p;

    list->sorted = 0;

    return HCORE_OK;
}

void
hcore_list_sort(hcore_list_t *list)
{
    // Validate arguments
    if (list == NULL || list->cmp == NULL)
    {
        return;
    }

    qsort(list->p, list->num_item, sizeof(void *),
          (int (*)(const void *, const void *))list->cmp);
    list->sorted = 1;
}

// Insert an item to the list
hcore_int_t
hcore_list_insert(hcore_list_t *list, void *p)
{
    int          low, high, middle;
    int          i;
    unsigned int old_num;
    unsigned int pos;

    // Validate arguments
    if (list == NULL || p == NULL)
    {
        return HCORE_ERROR;
    }

    if (list->cmp == NULL)
    {
        // adding simply if there is no sort function
        hcore_list_add(list, p);
        return HCORE_OK;
    }

    // Sort immediately if it is not sorted
    if (!list->sorted)
    {
        hcore_list_sort(list);
    }

    low  = 0;
    high = HCORE_LIST_NUM(list) - 1;

    pos = HCORE_INFINITE;

    while (low <= high)
    {
        int ret;

        middle = (low + high) / 2;
        ret    = list->cmp(&(list->p[middle]), &p);

        if (ret == 0)
        {
            pos = middle;
            break;
        }
        else if (ret > 0)
        {
            high = middle - 1;
        }
        else
        {
            low = middle + 1;
        }
    }

    if (pos == HCORE_INFINITE)
    {
        pos = low;
    }

    list->num_item++;
    if (list->num_item > list->num_reserved)
    {
        old_num = list->num_reserved;
        list->num_reserved *= 2;

        list->p =
            hcore_list_realloc(list->pool, list->p, sizeof(void *) * old_num,
                               sizeof(void *) * list->num_reserved);
    }

    if (HCORE_LIST_NUM(list) >= 2)
    {
        for (i = (HCORE_LIST_NUM(list) - 2); i >= (int)pos; i--)
        {
            list->p[i + 1] = list->p[i];
        }
    }

    list->p[pos] = p;

    return HCORE_OK;
}

hcore_int_t
hcore_list_delete(hcore_list_t *list, void *p)
{
    unsigned int i, n;
    unsigned int old_num;

    // Validate arguments
    if (list == NULL || p == NULL)
    {
        return HCORE_ERROR;
    }

    for (i = 0; i < list->num_item; i++)
    {
        if (list->p[i] == p)
        {
            break;
        }
    }
    if (i == list->num_item)
    {
        return HCORE_ERROR;
    }

    n = i;
    for (i = n; i < (list->num_item - 1); i++)
    {
        list->p[i] = list->p[i + 1];
    }

    list->num_item--;

    if ((list->num_item * 2) <= list->num_reserved)
    {
        if (list->num_reserved > (HCORE_LIST_INIT_NUM_RESERVED * 2))
        {
            old_num            = list->num_reserved;
            list->num_reserved = list->num_reserved / 2;

            list->p = hcore_list_realloc(list->pool, list->p,
                                         sizeof(void *) * old_num,
                                         sizeof(void *) * list->num_reserved);
        }
    }

    return HCORE_OK;
}

void
hcore_list_delete_all(hcore_list_t *list)
{
    unsigned int old_num;

    // Validate arguments
    if (list == NULL)
    {
        return;
    }

    old_num = list->num_reserved;

    list->num_item     = 0;
    list->num_reserved = HCORE_LIST_INIT_NUM_RESERVED;

    list->p = hcore_list_realloc(list->pool, list->p, sizeof(void *) * old_num,
                                 sizeof(void *) * HCORE_LIST_INIT_NUM_RESERVED);
}

// Search in the list
void *
hcore_list_search(hcore_list_t *list, void *target)
{
    void **ret;
    // Validate arguments
    if (list == NULL || target == NULL)
    {
        return NULL;
    }
    if (list->cmp == NULL)
    {
        return NULL;
    }

    // Check the sort
    if (list->sorted == 0)
    {
        // Sort because it is not sorted
        hcore_list_sort(list);
    }

    ret = (void **)bsearch(&target, list->p, list->num_item, sizeof(void *),
                           (int (*)(const void *, const void *))list->cmp);

    if (ret != NULL)
    {
        return *ret;
    }
    else
    {
        return NULL;
    }
}

void
hcore_deinit_list(hcore_list_t *list)
{
    if (list == NULL) return;

    if (list->pool == NULL)
    {
        free(list->p);
    }
}

void
hcore_destroy_list(hcore_list_t *list)
{
    if (list == NULL) return;

    hcore_deinit_list(list);

    if (list->pool == NULL)
    {
        free(list);
    }
}
