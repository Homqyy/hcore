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
#include <hcore_string.h>
#include <hcore_debug.h>

hcore_int_t
hcore_asnprintf(hcore_astring_t *astr, const char *fmt, ...)
{
    if (NULL == astr) { return HCORE_ERROR; }

    hcore_uchar_t *new_data;
    hcore_uchar_t *last;
    size_t       new_size = 0;

    while (1)
    {
        va_list args;
        va_start(args, fmt);
        last =
            hcore_vslprintf(astr->data + astr->len, astr->data + astr->size, fmt, args);
        va_end(args);

        if (last != astr->data + astr->size)
        {
            break;
        }

        new_size = astr->size * 2;
        new_data  = astr->realloc(astr->data, new_size);
        if (NULL == new_data)
        {
            astr->error = 1;
            return HCORE_ERROR;
        }

        astr->data = new_data;
        astr->size = new_size;
    }

    astr->len            = last - astr->data;
    astr->data[astr->len] = 0;

    return HCORE_OK;
}

hcore_int_t
hcore_init_astring(hcore_astring_t * astr, size_t size, 
    astr_realloc_pt astr_realloc, astr_free_pt astr_free)
{
    hcore_assert(astr);

    if (astr == NULL) return HCORE_ERROR;

    /* default value */
    if (size == 0) size = 64;
    if (astr_realloc == NULL) astr_realloc = realloc;
    if (astr_free == NULL) astr_free = free;

    hcore_uchar_t *data = astr_realloc(NULL, size);
    if (data == NULL)
    {
        return HCORE_ERROR;
    }

    astr->realloc = astr_realloc;
    astr->free = astr_free;
    astr->data = data;
    astr->len = 0;
    astr->size = size;
    astr->created = 0;
    astr->error = 0;

#ifdef _HCORE_DEBUG
    astr->inited = 1;
#endif
    return HCORE_OK;
}

hcore_astring_t *
hcore_create_astring(size_t size, 
    astr_realloc_pt astr_realloc, astr_free_pt astr_free)
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

void hcore_destroy_astring(hcore_astring_t * astr)
{
    if (astr == NULL) return;

#ifdef _HCORE_DEBUG
    hcore_assert(astr->inited);

    astr->inited = 0;
#endif

    astr->free(astr->data);

    if (astr->created)
    {
        astr->free(astr);
    }
}