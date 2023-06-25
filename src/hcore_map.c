/**
 * @file hcore_map.c
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

#include <hcore_constant.h>
#include <hcore_map.h>

#define HCORE_SET_MAP(m, bit)   ((m) |= 1 << bit)
#define HCORE_UNSET_MAP(m, bit) ((m) &= ~(1 << bit))

static hcore_int_t _hcore_map_set(hcore_map_t *map, hcore_uint_t pos,
                              hcore_bool_t is_set);

static u_char *_hcore_map_get_m(hcore_map_t *map, hcore_uint_t pos, hcore_uint_t *b);

static hcore_int_t _hcore_map_set_range(hcore_map_t *map, hcore_uint_t bp,
                                    hcore_uint_t ep, hcore_bool_t is_set);

hcore_map_t *
hcore_create_map(hcore_pool_t *pool, hcore_uint_t size)
{
    hcore_map_t *map;

    map = hcore_pcalloc(pool, sizeof(hcore_map_t));

    if (map == NULL) return NULL;

    if (hcore_init_map(map, pool, size) != HCORE_OK) return NULL;

    return map;
}

hcore_int_t
hcore_init_map(hcore_map_t *map, hcore_pool_t *pool, hcore_uint_t size)
{
    if (!map) return HCORE_ERROR;

    map->pool = pool;
    map->n    = size / 8 + !!(size % 8);
    map->size = size;
    map->m    = hcore_pcalloc(map->pool, map->n);

    if (map->m == NULL) return HCORE_ERROR;

    return HCORE_OK;
}

static u_char *
_hcore_map_get_m(hcore_map_t *map, hcore_uint_t pos, hcore_uint_t *b)
{
    hcore_uint_t mp, bit;

    if (!map || !pos || map->size < pos) return NULL;

    pos--; // convert 'pos' to index of array

    mp  = pos / 8;
    bit = pos % 8;

    if (map->n <= mp || 7 < bit) return NULL;

    *b = bit;

    return &map->m[mp];
}

static hcore_int_t
_hcore_map_set(hcore_map_t *map, hcore_uint_t pos, hcore_bool_t is_set)
{
    hcore_uint_t bit;
    u_char *   m;

    m = _hcore_map_get_m(map, pos, &bit);

    if (m == NULL) return HCORE_ERROR;

    if (is_set)
        HCORE_SET_MAP(*m, bit);
    else
        HCORE_UNSET_MAP(*m, bit);

    return HCORE_OK;
}

hcore_int_t
hcore_map_set(hcore_map_t *map, hcore_uint_t pos)
{
    return _hcore_map_set(map, pos, 1);
}

static hcore_int_t
_hcore_map_set_range(hcore_map_t *map, hcore_uint_t bp, hcore_uint_t ep,
                   hcore_bool_t is_set)
{
    u_char *   bm, *em, *p;
    hcore_uint_t b_bit, e_bit, i;

    bm = _hcore_map_get_m(map, bp, &b_bit);

    if (!bm) return HCORE_ERROR;

    em = _hcore_map_get_m(map, ep, &e_bit);

    if (!em) return HCORE_ERROR;

    if (bm != em)
    {
        for (i = b_bit; i < 8; i++)
            is_set ? HCORE_SET_MAP(*bm, i) : HCORE_UNSET_MAP(*bm, i);

        for (i = 0; i <= e_bit; i++)
            is_set ? HCORE_SET_MAP(*em, i) : HCORE_UNSET_MAP(*em, i);
    }
    else
    {
        for (i = b_bit; i <= e_bit; i++)
            is_set ? HCORE_SET_MAP(*bm, i) : HCORE_UNSET_MAP(*bm, i);
    }

    p = bm + 1;
    while (p < em) { *p++ = 0xff; }

    return HCORE_OK;
}

hcore_int_t
hcore_map_set_range(hcore_map_t *map, hcore_uint_t bp, hcore_uint_t ep)
{
    return _hcore_map_set_range(map, bp, ep, 1);
}

hcore_int_t
hcore_map_unset(hcore_map_t *map, hcore_uint_t pos)
{
    return _hcore_map_set(map, pos, 0);
}

hcore_int_t
hcore_map_unset_range(hcore_map_t *map, hcore_uint_t bp, hcore_uint_t ep)
{
    return _hcore_map_set_range(map, bp, ep, 0);
}

hcore_int_t
hcore_map_is_set(hcore_map_t *map, hcore_uint_t pos)
{
    hcore_uint_t bit;
    u_char *   m;

    m = _hcore_map_get_m(map, pos, &bit);

    if (m == NULL) return HCORE_ERROR;

    return *m & (1 << bit);
}