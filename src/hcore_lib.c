/**
 * @file hcore_lib.c
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
#include <hcore_string.h>

#include <stdlib.h>

void *
hcore_malloc(size_t size)
{
#ifdef _HCORE_DEBUG
    hcore_debug_mnode_t *node = hcore_debug_create_mnode("hcore_malloc", size);
    if (node == NULL) return NULL;

    return node->addr;
#else
    return malloc(size);
#endif
}

void *
hcore_calloc(size_t count, size_t size)
{
#ifdef _HCORE_DEBUG
    hcore_debug_mnode_t *node =
        hcore_debug_create_mnode("hcore_calloc", count * size);
    if (node == NULL) return NULL;

    memset(node->addr, 0x00, count * size);

    return node->addr;
#else
    return calloc(count, size);
#endif
}

void *
hcore_realloc(void *ptr, size_t size)
{
#ifdef _HCORE_DEBUG
    // create new
    hcore_debug_mnode_t *new_node =
        hcore_debug_create_mnode("hcore_realloc", size);
    if (new_node == NULL) return NULL;

    if (ptr)
    {
        hcore_debug_mnode_t *old_node = hcore_debug_get_mnode_of_addr(ptr);

        // copy old to new

        size_t copy_size = hcore_min(new_node->size, old_node->size);

        hcore_memcpy(new_node + 1, old_node + 1, copy_size);

        // delete old
        hcore_debug_destroy_mnode(old_node);
    }

    return new_node->addr;
#else
    return realloc(ptr, size);
#endif
}

void
hcore_free(void *ptr)
{
#ifdef _HCORE_DEBUG
    hcore_debug_mnode_t *node = hcore_debug_get_mnode_of_addr(ptr);
    hcore_debug_destroy_mnode(node);
#else
    free(ptr);
#endif
}