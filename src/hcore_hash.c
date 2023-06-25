/**
 * @file hcore_hash.c
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

#include <hcore_hash.h>
#include <hcore_types.h>

hcore_uint_t
hcore_hash_key(hcore_uchar_t *data, size_t len)
{
    hcore_uint_t i, key;

    key = 0;

    for (i = 0; i < len; i++) { key = hcore_hash(key, data[i]); }

    return key;
}