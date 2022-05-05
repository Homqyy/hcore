/**
 * @file hcore_path.c
 * @author diyao (cauc.peter@gmail.com)
 * @brief
 * @version 0.1
 * @date 2021-09-16
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#include <hcore_debug.h>
#include <hcore_path.h>

hcore_str_t *
hcore_get_full_path(hcore_pool_t *pool, const char *src, hcore_str_t *def_path)
{
    hcore_str_t *dst;
    hcore_str_t  prefix = *def_path;

    hcore_assert(pool && src && def_path);

    if (hcore_is_abspath(src) || def_path->len == 0) { hcore_str_set(&prefix, ""); }

    if (src) dst = hcore_pnalloc(pool, sizeof(hcore_str_t));
    if (dst == NULL) return NULL;

    dst->len  = def_path->len + 1 /* '/' */ + strlen(src);
    dst->data = hcore_pnalloc(pool, dst->len + 1 /* '\0' */);
    if (dst->data == NULL) return NULL;

    if (prefix.len)
    {
        hcore_snprintf(dst->data, dst->len + 1, "%V/%s%Z", &prefix, src);
    }
    else
    {
        hcore_snprintf(dst->data, dst->len + 1, "%s%Z", src);
    }

    return dst;
}
