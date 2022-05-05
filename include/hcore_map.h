/**
 * @file hcore_map.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief
 * @version 0.1
 * @date 2021-11-08
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#ifndef _HCORE_MAP_H_INCLUDED_
#define _HCORE_MAP_H_INCLUDED_

#include <hcore_pool.h>

typedef struct
{
    hcore_pool_t *pool;
    u_char *    m;    // unit of map is 8 bit
    hcore_uint_t  n;    // number of map
    hcore_uint_t  size; // size of initialized
} hcore_map_t;

hcore_map_t *hcore_create_map(hcore_pool_t *pool, hcore_uint_t size);
hcore_int_t  hcore_init_map(hcore_map_t *map, hcore_pool_t *pool, hcore_uint_t size);
hcore_int_t  hcore_map_set(hcore_map_t *map, hcore_uint_t pos);
hcore_int_t  hcore_map_set_range(hcore_map_t *map, hcore_uint_t bp, hcore_uint_t ep);
hcore_int_t  hcore_map_unset(hcore_map_t *map, hcore_uint_t pos);
hcore_int_t  hcore_map_unset_range(hcore_map_t *map, hcore_uint_t bp, hcore_uint_t ep);
hcore_int_t  hcore_map_is_set(hcore_map_t *map, hcore_uint_t pos);

#endif // !_HCORE_MAP_H_INCLUDED_
