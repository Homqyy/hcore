/**
 * @file hcore_crc32.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#ifndef _HCORE_CRC32_H_INCLUDED_
#define _HCORE_CRC32_H_INCLUDED_

#include <hcore_types.h>

extern hcore_uint32_t *hcore_crc32_table_short;
extern hcore_uint32_t  hcore_crc32_table256[];


static inline hcore_uint32_t
hcore_crc32_short(hcore_uchar_t *p, size_t len)
{
    hcore_uchar_t  c;
    hcore_uint32_t crc;

    crc = 0xffffffff;

    while (len--)
    {
        c   = *p++;
        crc = hcore_crc32_table_short[(crc ^ (c & 0xf)) & 0xf] ^ (crc >> 4);
        crc = hcore_crc32_table_short[(crc ^ (c >> 4)) & 0xf] ^ (crc >> 4);
    }

    return crc ^ 0xffffffff;
}


static inline hcore_uint32_t
hcore_crc32_long(hcore_uchar_t *p, size_t len)
{
    hcore_uint32_t crc;

    crc = 0xffffffff;

    while (len--)
    {
        crc = hcore_crc32_table256[(crc ^ *p++) & 0xff] ^ (crc >> 8);
    }

    return crc ^ 0xffffffff;
}


#define hcore_crc32_init(crc) crc = 0xffffffff


static inline void
hcore_crc32_update(hcore_uint32_t *crc, hcore_uchar_t *p, size_t len)
{
    hcore_uint32_t c;

    c = *crc;

    while (len--)
    {
        c = hcore_crc32_table256[(c ^ *p++) & 0xff] ^ (c >> 8);
    }

    *crc = c;
}


#define hcore_crc32_final(crc) crc ^= 0xffffffff


hcore_int_t hcore_crc32_table_init(void);

#endif //!_HCORE_CRC32_H_INCLUDED_