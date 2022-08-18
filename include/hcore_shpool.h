/**
 * @file hcore_shpool.h
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

#ifndef _HCORE_SHPOOL_H_INCLUDED_
#define _HCORE_SHPOOL_H_INCLUDED_

#include <hcore_log.h>
#include <hcore_shmtx.h>
#include <hcore_types.h>

#include <stddef.h>

typedef struct hcore_slab_page_s hcore_slab_page_t;

struct hcore_slab_page_s
{
    uintptr_t          slab; // slab size (small) or bitmap (exact or big)
    uintptr_t          prev;
    hcore_slab_page_t *next;
};

typedef struct
{
    hcore_shmtx_sh_t   lock;
    /*
     * to express minimum size(this.min_size) of allocated space with shift */
    size_t             min_shift;
    size_t             min_size;
    hcore_slab_page_t *pages;
    hcore_slab_page_t *last; // last page that can be allocated by the pool
    hcore_slab_page_t  free; // header of free list

    hcore_uint_t pfree; // free pages

    hcore_uchar_t *addr;  // start address of the whole shared memory
    hcore_uchar_t *start; // start address that can be allocated by the pool
    hcore_uchar_t *end;   // end address that the whole shared memory
} hcore_slab_pool_t;

typedef struct
{
    hcore_log_t       *log;
    hcore_uchar_t     *addr; // start address of the whole shared memory
    hcore_slab_pool_t *sp;   // equal to this.addr
    size_t             size; // size of whole shared memory
    hcore_shmtx_t      mutex;
    const char        *name; // name of shpool, it's must string of constant

    hcore_uint_t create : 1;
} hcore_shpool_t;

/**
 * @brief create a pool of shared memory
 *
 * @param log log object
 * @param name name of pool, it should be unique and constant. Of course, name
 * can be 'NULL' that meant what the pool is anonymous, so you can't again get
 * it by any way.
 * @param size expecting size of pool
 * @return hcore_shpool_t* : Upon successful is return a pool object, otherwise
 * return NULL
 */
hcore_shpool_t *hcore_create_shpool(hcore_log_t *log, const char *name,
                                    size_t size);
hcore_shpool_t *hcore_get_shpool(hcore_log_t *log, const char *name);
/**
 * @brief destroy a pool object
 *
 * @param shpool pool object
 */
void            hcore_destroy_shpool(hcore_shpool_t *shpool);

void *hcore_shpool_alloc(hcore_shpool_t *shpool, size_t size);
void *hcore_shpool_alloc_locked(hcore_shpool_t *shpool, size_t size);
void *hcore_shpool_calloc(hcore_shpool_t *shpool, size_t size);
void  hcore_shpool_free(hcore_shpool_t *shpool, void *p);
void  hcore_shpool_free_locked(hcore_shpool_t *shpool, void *p);

#define hcore_shpool_lock(shpool)    hcore_shmtx_lock(&(shpool)->mutex)
#define hcore_shpool_unlock(shpool)  hcore_shmtx_unlock(&(shpool)->mutex)
#define hcore_shpool_trylock(shpool) hcore_shmtx_trylock(&(shpool)->mutex)

#ifdef _HCORE_DEBUG
void hcore_shpool_dump(hcore_shpool_t *shpool);
#else
#define hcore_shpool_dump(shpool)
#endif

#endif //!_HCORE_SHPOOL_H_INCLUDED_