/**
 * @file hcore_base.c
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief
 * @version 0.1
 * @date 2022-08-12
 *
 * @copyright Copyright (c) 2022 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#include <hcore_base.h>
#include <hcore_types.h>

#include <unistd.h>

static hcore_int_t g_hcore_pagesize         = -1;
static hcore_int_t g_hcore_pagesize_shift   = -1;
static hcore_int_t g_hcore_slab_max_size    = -1;
static hcore_int_t g_hcore_slab_exact_size  = -1;
static hcore_int_t g_hcore_slab_exact_shift = -1;

static void hcore_init_slab(void);

void
hcore_process_invoke_once(void)
{
    hcore_getpagesize();
    hcore_getpagesize_shift();
    hcore_getpid();

    hcore_init_slab();
}

static void
hcore_init_slab(void)
{
    hcore_int_t n;

    g_hcore_slab_max_size = hcore_getpagesize() / 2;
    g_hcore_slab_exact_size =
        hcore_getpagesize() / (8 * sizeof(uintptr_t) /* bits */);

    for (n = g_hcore_slab_exact_size; n >>= 1; g_hcore_slab_exact_shift++)
    {
        // nothing
    }
}

hcore_int_t
hcore_get_slab_max_size(void)
{
    if (g_hcore_slab_max_size == -1) hcore_init_slab();

    return g_hcore_slab_max_size;
}

hcore_int_t
hcore_get_slab_exact_size(void)
{
    if (g_hcore_slab_exact_size == -1) hcore_init_slab();

    return g_hcore_slab_exact_size;
}

hcore_int_t
hcore_get_slab_exact_shift(void)
{
    if (g_hcore_slab_exact_shift == -1) hcore_init_slab();

    return g_hcore_slab_exact_shift;
}

hcore_int_t
hcore_getpagesize(void)
{
    if (g_hcore_pagesize == -1) g_hcore_pagesize = getpagesize();

    return g_hcore_pagesize;
}

hcore_int_t
hcore_getpagesize_shift(void)
{
    hcore_int_t pagesize = hcore_getpagesize();

    if (g_hcore_pagesize_shift == -1)
    {
        hcore_int_t n;
        for (n = g_hcore_pagesize; n >>= 1; g_hcore_pagesize_shift++)
        {
            // nothing
        }
    }

    return g_hcore_pagesize_shift;
}

hcore_pid_t
hcore_getpid(void)
{
    return getpid();
}