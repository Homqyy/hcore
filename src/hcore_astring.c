/**
 * @file hcore_astring.c
 * @author fang_xing (fang_xing@topesc.com.cn)
 * @brief
 * @version 0.1
 * @date 2021-11-04
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#include <hcore_astring.h>
#include <hcore_debug.h>
#include <hcore_string.h>

hcore_int_t
hcore_astrfmt_size(hcore_astring_t *astr, ssize_t size)
{
    const char  *units[] = {"B", "KB", "MB", "GB", "TB", NULL};
    const char **pu;
    ssize_t      scale, v;
    double       decimal;
    double       r; // remainder

    pu    = units;
    v     = size;
    scale = 1;

    while (v)
    {
        if (!(v >> 10)) break;

        pu++;
        scale <<= 10;
        v >>= 10;
    }

    r = size & (scale - 1);

    decimal = r / scale;
    decimal += v;

    return hcore_asnprintf(astr, "%.02f %s", decimal, *pu);
}

hcore_int_t
hcore_asnprintf(hcore_astring_t *astr, const char *fmt, ...)
{
    hcore_assert(astr);

    if (NULL == astr || NULL == fmt)
    {
        return HCORE_ERROR;
    }

    hcore_uchar_t *new_data;
    hcore_uchar_t *last;
    size_t         new_size = 0;

    while (1)
    {
        va_list args;
        va_start(args, fmt);
        last = hcore_vslprintf(astr->data + astr->len, astr->data + astr->size,
                               fmt, args);
        va_end(args);

        if (last != astr->data + astr->size)
        {
            break;
        }

        new_size = astr->size * 2;
        new_data = astr->realloc(astr->data, new_size);
        if (NULL == new_data)
        {
            astr->error = 1;
            return HCORE_ERROR;
        }

        astr->data = new_data;
        astr->size = new_size;
    }

    astr->len             = last - astr->data;
    astr->data[astr->len] = 0;

    return HCORE_OK;
}

hcore_int_t
hcore_init_astring(hcore_astring_t *astr, size_t size,
                   astr_realloc_pt astr_realloc, astr_free_pt astr_free)
{
    hcore_assert(astr);

    if (astr == NULL) return HCORE_ERROR;

    /* default value */
    if (size == 0) size = HCORE_ASTRING_DEF_SIZE;
    if (astr_realloc == NULL) astr_realloc = realloc;
    if (astr_free == NULL) astr_free = free;

    hcore_uchar_t *data = astr_realloc(NULL, size);
    if (data == NULL)
    {
        return HCORE_ERROR;
    }

    astr->realloc = astr_realloc;
    astr->free    = astr_free;
    astr->data    = data;
    astr->len     = 0;
    astr->size    = size;
    astr->created = 0;
    astr->error   = 0;
    astr->inited  = 1;

    return HCORE_OK;
}

hcore_astring_t *
hcore_create_astring(size_t size, astr_realloc_pt astr_realloc,
                     astr_free_pt astr_free)
{
    if (astr_realloc == NULL) astr_realloc = realloc;
    if (astr_free == NULL) astr_free = free;

    hcore_astring_t *astr = astr_realloc(NULL, sizeof(hcore_astring_t));
    if (astr == NULL) return NULL;

    if (hcore_init_astring(astr, size, astr_realloc, astr_free) == HCORE_ERROR)
    {
        astr_free(astr);
        return NULL;
    }

    astr->created = 1;

    return astr;
}

void
hcore_destroy_astring(hcore_astring_t *astr)
{
    hcore_assert(astr && astr->inited);

    if (astr == NULL || astr->inited == 0) return;

    hcore_assert(astr->inited);

    astr->inited = 0;

    astr->free(astr->data);

    if (astr->created)
    {
        astr->free(astr);
    }
}