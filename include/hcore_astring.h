/**
 * @file hcore_astring.h
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

#ifndef _HCORE_ASTRING_H_INCLUDE_
#define _HCORE_ASTRING_H_INCLUDE_

#include <hcore_types.h>

#include <stdio.h>
#include <stdlib.h>

typedef void *(*astr_realloc_pt)(void *ptr, size_t size);
typedef void (*astr_free_pt)(void *ptr);

typedef struct
{
    hcore_uchar_t *data;
    size_t         size; /* alloced size */
    size_t         len;  /* used size */

    astr_realloc_pt realloc; // default is realloc of libc
    astr_free_pt    free;    // default is free of libc

    hcore_uint_t error   : 1;
    hcore_uint_t created : 1;
#ifdef _HCORE_DEBUG
    hcore_uint_t inited : 1;
#endif
} hcore_astring_t;

/*
 * brief:按照fmt的格式格式字符串并将其拷贝到str中，可以动态扩展空间
 * param: *str
 * param:格式字符串的格式
 * ret
 *  成功返回0
 *  失败返回-1
 */

hcore_int_t      hcore_asnprintf(hcore_astring_t *astr, const char *fmt, ...);
hcore_astring_t *hcore_create_astring(size_t size, astr_realloc_pt astr_realloc,
                                      astr_free_pt astr_free);
hcore_int_t      hcore_init_astring(hcore_astring_t *astr, size_t size,
                                    astr_realloc_pt astr_realloc,
                                    astr_free_pt    astr_free);
void             hcore_destroy_astring(hcore_astring_t *astr);

#define hcore_astring_get_data(astr) (astr)->data
#define hcore_astring_get_len(astr)  (astr)->len
#define hcore_astring_is_error(astr) (astr)->error

#endif // !_HCORE_ASTRING_H_INCLUDE_
