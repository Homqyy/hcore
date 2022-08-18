/**
 * @file hcore_shpool.c
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
#include <hcore_constant.h>
#include <hcore_debug.h>
#include <hcore_lib.h>
#include <hcore_shmtx.h>
#include <hcore_shpool.h>
#include <hcore_string.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>


#define HCORE_SLAB_PAGE_FREE  0
#define HCORE_SLAB_PAGE_BUSY  0xffffffffffffffff
#define HCORE_SLAB_PAGE_START 0x8000000000000000

#define HCORE_SLAB_SHIFT_MASK 0x000000000000000f
#define HCORE_SLAB_MAP_SHIFT  32
#define HCORE_SLAB_MAP_MASK   0xffffffff00000000

#define HCORE_SLAB_BUSY 0xffffffffffffffff

#define HCORE_SLAB_PAGE_MASK 3
#define HCORE_SLAB_PAGE      0
#define HCORE_SLAB_BIG       1
#define HCORE_SLAB_EXACT     2
#define HCORE_SLAB_SMALL     3

#define HCORE_SLAB_MIN_SIZE(pagesize) (pagesize << 3)

#define hcore_slab_get_slots(pool) ((hcore_slab_page_t *)((pool) + 1))

#define hcore_slab_get_page_type(page) ((page)->prev & HCORE_SLAB_PAGE_MASK)

#define hcore_slab_get_page_addr(pool, page, pagesize_shift) \
    ((((page) - (pool)->pages) << pagesize_shift) + (uintptr_t)(pool)->start)

#define hcore_slab_get_page_prev(page) \
    (hcore_slab_page_t *)((page)->prev & ~HCORE_SLAB_PAGE_MASK)
#define hcore_slab_set_page_prev(page, prev_page, type) \
    ((page)->prev = (uintptr_t)(prev_page) | (type))


#define hcore_slab_remove_full_page(page, type)            \
    {                                                      \
        hcore_slab_page_t *prev;                           \
                                                           \
        /* the page is 100% used, remove from page list */ \
        prev             = hcore_slab_get_page_prev(page); \
        prev->next       = page->next;                     \
        page->next->prev = page->prev;                     \
                                                           \
        /* indicate to 100% used, only set type */         \
        page->next = NULL;                                 \
        hcore_slab_set_page_prev(page, 0, type);           \
    }


static void               hcore_slab_init(hcore_slab_pool_t *pool);
static hcore_slab_page_t *hcore_slab_alloc_pages(hcore_slab_pool_t *pool,
                                                 hcore_uint_t alloc_page_n);
static void               hcore_slab_free_pages(hcore_slab_pool_t *pool,
                                                hcore_slab_page_t *page, hcore_int_t page_n);
static void  hcore_slab_free_locked(hcore_slab_pool_t *pool, void *p);
static void *hcore_slab_alloc_locked(hcore_slab_pool_t *pool, size_t size);

hcore_shpool_t *
hcore_create_shpool(hcore_log_t *log, const char *name, size_t size)
{
    hcore_shpool_t *shpool = NULL;
    int             flags  = MAP_SHARED;
    int             fd     = -1;

    hcore_assert(log && size);

    if (log == NULL || size == 0) return NULL;

    shpool = hcore_malloc(sizeof(hcore_shpool_t));
    if (shpool == NULL) return NULL;

    hcore_memzero(shpool, sizeof(hcore_shpool_t));

    if (name == NULL)
    {
        fd = -1;
        flags |= MAP_ANON;
    }
    else
    {
        fd = shm_open(name, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd == -1)
        {
            hcore_log_error(
                HCORE_LOG_ALERT, log, errno,
                "shm_open(%s, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR) failed", name);
            goto error;
        }
    }

    // count size
    hcore_int_t pagesize = hcore_getpagesize();

    size = hcore_max(size, HCORE_SLAB_MIN_SIZE(pagesize));

    if (fd != -1) ftruncate(fd, size);

    // create pool of shared memory
    shpool->addr = mmap(NULL, size, PROT_READ | PROT_WRITE, flags, fd, 0);
    if (shpool->addr == MAP_FAILED)
    {
        hcore_log_error(HCORE_LOG_ALERT, log, errno,
                        "mmap(MAP_SHARED%s, %uz) failed",
                        flags & MAP_ANON ? "|MAP_ANON" : "", size);
        goto error;
    }

    shpool->log    = log;
    shpool->name   = name;
    shpool->size   = size;
    shpool->create = 1;

    hcore_slab_pool_t *sp = (hcore_slab_pool_t *)shpool->addr;

    sp->addr      = shpool->addr;
    sp->end       = shpool->addr + shpool->size;
    sp->min_shift = 3;

    if (hcore_shmtx_init(&shpool->mutex, &sp->lock) != HCORE_OK)
    {
        goto error;
    }

    hcore_slab_init(sp);

    shpool->sp = sp;

    if (fd != -1) close(fd);

    return shpool;

error:
    if (shpool)
    {
        if (shpool->addr != MAP_FAILED)
        {
            if (munmap(shpool->addr, shpool->size) == -1)
            {
                hcore_log_error(HCORE_LOG_ALERT, log, errno,
                                "munmap(%p, %uz) failed", shpool->addr,
                                shpool->size);
            }
        }

        hcore_free(shpool);
    }

    if (fd != -1)
    {
        if (close(fd) == -1)
        {
            hcore_log_error(HCORE_LOG_ALERT, log, errno, "close(#%d) failed",
                            fd);
        }

        if (shm_unlink(name) == -1)
        {
            hcore_log_error(HCORE_LOG_ALERT, log, errno,
                            "shm_unlink(%s) failed", name);
        }
    }

    return NULL;
}

hcore_shpool_t *
hcore_get_shpool(hcore_log_t *log, const char *name)
{
    hcore_shpool_t *shpool = NULL;
    int             flags  = MAP_SHARED;
    int             fd     = -1;

    hcore_assert(log && name);

    if (log == NULL || name == NULL) return NULL;

    shpool = hcore_malloc(sizeof(hcore_shpool_t));
    if (shpool == NULL) return NULL;

    hcore_memzero(shpool, sizeof(hcore_shpool_t));

    fd = shm_open(name, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        hcore_log_error(HCORE_LOG_ALERT, log, errno,
                        "shm_open(%s, O_RDWR, S_IRUSR | S_IWUSR) failed", name);
        goto error;
    }

    // get size

    struct stat buf;

    if (fstat(fd, &buf) == -1)
    {
        hcore_log_error(HCORE_LOG_ALERT, log, errno, "fstat(#%d) failed", fd);
        goto error;
    }

    size_t size = buf.st_size;

    shpool->addr = mmap(NULL, size, PROT_READ | PROT_WRITE, flags, fd, 0);
    if (shpool->addr == MAP_FAILED)
    {
        hcore_log_error(HCORE_LOG_ALERT, log, errno,
                        "mmap(MAP_SHARED, %uz) failed", size);
        goto error;
    }

    // init shpool

    shpool->log  = log;
    shpool->name = name;
    shpool->size = size;

    hcore_slab_pool_t *sp = (hcore_slab_pool_t *)shpool->addr;

    if (hcore_shmtx_init(&shpool->mutex, &sp->lock) != HCORE_OK)
    {
        goto error;
    }

    shpool->sp = sp;

    close(fd);

    return shpool;

error:
    if (shpool)
    {
        if (shpool->addr != MAP_FAILED)
        {
            if (munmap(shpool->addr, shpool->size) == -1)
            {
                hcore_log_error(HCORE_LOG_ALERT, log, errno,
                                "munmap(%p, %uz) failed", shpool->addr,
                                shpool->size);
            }
        }

        hcore_free(shpool);
    }

    if (fd != -1)
    {
        if (close(fd) == -1)
        {
            hcore_log_error(HCORE_LOG_ALERT, log, errno, "close(#%d) failed",
                            fd);
        }

        if (shm_unlink(name) == -1)
        {
            hcore_log_error(HCORE_LOG_ALERT, log, errno,
                            "shm_unlink(%s) failed", name);
        }
    }

    return NULL;
}

/*
 * internal construction : | sizeof(hcore_slab_pool_t) | max_slab_n *
 * sizeof(hcore_slab_page_t) | n * sizeof(hcore_slab_page_t) | align_padding
 * | n * page |
 *
 * max_slab_n : pagesize_shift - this.min_shift
 *
 */
static void
hcore_slab_init(hcore_slab_pool_t *pool)
{
    hcore_uchar_t     *p;
    size_t             size;
    hcore_slab_page_t *slots;
    hcore_uint_t       n;
    hcore_int_t        pagesize = hcore_getpagesize();

    pool->min_size = (size_t)1 << pool->min_shift;
    slots          = hcore_slab_get_slots(pool);

    p = (hcore_uchar_t *)slots;

    n = hcore_getpagesize_shift() - pool->min_shift;

    // slot of slab as a list head
    hcore_uint_t i;
    for (i = 0; i < n; i++)
    {
        /*
         * only "next" is used in list head */
        slots[i].slab = 0;
        slots[i].next = &slots[i];
        slots[i].prev = 0;
    }

    p += n * sizeof(hcore_slab_page_t); // skip headers of slab

    size = pool->end - p; // size of all pages

    /*
     * whole page = header + page; */
    hcore_uint_t page_n =
        (hcore_uint_t)(size / (pagesize + sizeof(hcore_slab_page_t)));

    /*
     * type of all pages is be set '0(HCORE_SLAB_PAGE)' by 'hcore_memzero' */
    pool->pages = (hcore_slab_page_t *)p;
    hcore_memzero(pool->pages, page_n * sizeof(hcore_slab_page_t));

    hcore_slab_page_t *page = pool->pages;

    /*
     * only "next" is used in list head */
    pool->free.slab = 0;
    pool->free.next = page;
    pool->free.prev = 0;

    page->slab = page_n;
    page->next = &pool->free;
    page->prev = (uintptr_t)&pool->free;

    // multiples of pagesize
    pool->start =
        hcore_align_ptr(p + page_n * sizeof(hcore_slab_page_t), pagesize);

    // update pages number
    hcore_int_t m = page_n - (pool->end - pool->start) / pagesize;
    if (0 < m)
    {
        page_n -= m;
        page->slab = page_n;
    }

    pool->last  = pool->pages + page_n;
    pool->pfree = page_n;
}


static void
hcore_slab_free_locked(hcore_slab_pool_t *pool, void *p)
{
    if ((hcore_uchar_t *)p < pool->start || (hcore_uchar_t *)p > pool->end)
    {
        goto failed;
    }

    hcore_int_t        pagesize    = hcore_getpagesize();
    hcore_int_t        exact_size  = hcore_get_slab_exact_size();
    hcore_int_t        exact_shift = hcore_get_slab_exact_shift();
    hcore_uint_t       i, type;
    hcore_slab_page_t *page;
    uintptr_t          slab, m;

    i    = ((hcore_uchar_t *)p - pool->start) >> pagesize;
    page = &pool->pages[i];
    slab = page->slab;
    type = hcore_slab_get_page_type(page);

    hcore_uint_t       shift, slot, n;
    size_t             size;
    uintptr_t         *bitmap;
    hcore_slab_page_t *slots;

    switch (type)
    {
    case HCORE_SLAB_SMALL:
        shift = slab & HCORE_SLAB_SHIFT_MASK;
        size  = (size_t)1 << shift;

        if ((uintptr_t)p & (size - 1))
        {
            goto wrong_chunk;
        }

        // count position in map
        i = ((uintptr_t)p & (pagesize - 1)) >> shift;
        m = (uintptr_t)1 << (i % (8 * sizeof(uintptr_t)));
        i /= 8 * sizeof(uintptr_t);

        bitmap = (uintptr_t *)((uintptr_t)p & ~((uintptr_t)pagesize - 1));

        if (bitmap[i] & m)
        {
            slot = shift - pool->min_shift;

            if (page->next == NULL)
            {
                // insert to page list

                slots = hcore_slab_get_slots(pool);

                page->next       = slots[slot].next;
                slots[slot].next = page;

                hcore_slab_set_page_prev(page, &slots[slot], HCORE_SLAB_SMALL);
                hcore_slab_set_page_prev(page->next, page, HCORE_SLAB_SMALL);
            }

            bitmap[i] &= ~m; // unset bit

            n = (pagesize >> shift) / ((1 << shift) * 8);

            if (n == 0) n = 1;

            i = n / (8 * sizeof(uintptr_t));
            m = ((uintptr_t)1 << (n % (8 * sizeof(uintptr_t)))) - 1;

            // free empty page

            if (bitmap[i] & ~m)
            {
                goto done; // no empty in current map
            }

            hcore_uint_t map_n;

            map_n = (pagesize >> shift) / (8 * sizeof(uintptr_t));

            for (i = i + 1; i < map_n; i++)
            {
                if (bitmap[i]) goto done; // all no empty in other maps
            }

            hcore_slab_free_pages(pool, page, 1); // free empty page

            goto done;
        }

        goto chunk_already_free;

    case HCORE_SLAB_EXACT:
        m    = (uintptr_t)1 << (((uintptr_t)p & (pagesize - 1)) >> exact_shift);
        size = exact_size;
        if ((uintptr_t)p & (size - 1)) goto wrong_chunk;

        if (slab & m)
        {
            slot = exact_shift - pool->min_shift;

            if (slab == HCORE_SLAB_BUSY)
            {
                // insert to page list

                slots = hcore_slab_get_slots(pool);

                page->next       = slots[slot].next;
                slots[slot].next = page;

                hcore_slab_set_page_prev(page, &slots[slot], HCORE_SLAB_EXACT);
                hcore_slab_set_page_prev(page->next, page, HCORE_SLAB_EXACT);
            }

            page->slab &= ~m; // unset bit

            // free empty page

            if (page->slab)
            {
                goto done;
            }

            hcore_slab_free_pages(pool, page, 1);

            goto done;
        }

        goto chunk_already_free;

    case HCORE_SLAB_BIG:
        shift = slab & HCORE_SLAB_SHIFT_MASK;
        size  = (size_t)1 << shift;

        if ((uintptr_t)p & (size - 1)) goto wrong_chunk;

        m = (uintptr_t)1 << ((((uintptr_t)p & (pagesize - 1)) >> shift)
                             + HCORE_SLAB_MAP_SHIFT);

        if (slab & m)
        {
            slot = shift - pool->min_shift;

            if (page->next == NULL)
            {
                // insert to page list

                slots = hcore_slab_get_slots(pool);

                page->next       = slots[slot].next;
                slots[slot].next = page;

                hcore_slab_set_page_prev(page, &slots[slot], HCORE_SLAB_BIG);
                hcore_slab_set_page_prev(page->next, page, HCORE_SLAB_BIG);
            }

            page->slab &= ~m; // unset bit

            // free empty page

            if (page->slab & HCORE_SLAB_MAP_MASK)
            {
                goto done;
            }

            hcore_slab_free_pages(pool, page, 1);

            goto done;
        }

        goto chunk_already_free;

    case HCORE_SLAB_PAGE:
        if ((uintptr_t)p & (pagesize - 1)) goto wrong_chunk;

        if (!(slab & HCORE_SLAB_PAGE_START))
        {
            goto failed; // already free
        }

        if (slab == HCORE_SLAB_PAGE_BUSY)
        {
            goto failed; // point to wrong page
        }

        size = slab & ~HCORE_SLAB_PAGE_START;

        hcore_slab_free_pages(pool, page, size);

        return;
    }

    /* not reached */

    hcore_bug_on();

done:
wrong_chunk:
chunk_already_free:
failed:
    return;
}

static void *
hcore_slab_alloc_locked(hcore_slab_pool_t *pool, size_t size)
{
    hcore_int_t        pagesize       = hcore_getpagesize();
    hcore_int_t        pagesize_shift = hcore_getpagesize_shift();
    hcore_int_t        exact_size     = hcore_get_slab_exact_size();
    hcore_int_t        exact_shift    = hcore_get_slab_exact_shift();
    hcore_slab_page_t *page;
    uintptr_t          p;

    if (hcore_get_slab_max_size() < size)
    {
        page = hcore_slab_alloc_pages(pool, (size >> pagesize_shift)
                                                + ((size % pagesize) ? 1 : 0));

        if (page)
        {
            p = hcore_slab_get_page_addr(pool, page, pagesize_shift);
        }
        else
        {
            p = 0;
        }

        goto done;
    }

    hcore_int_t  shift;
    hcore_uint_t slot;

    if (pool->min_size < size)
    {
        shift = 1; // include remainder
        for (size_t s = size - 1
             /* To avoid to exceed need mininum size when no remainder */
             ;
             s >>= 1; shift++)
        {
            // nothing
        }

        slot = shift - pool->min_shift;
    }
    else
    {
        shift = pool->min_shift;
        slot  = 0;
    }

    hcore_slab_page_t *slots, *prev;
    uintptr_t         *bitmap, m;
    hcore_uint_t       map_n, i, bit_n;

    slots = hcore_slab_get_slots(pool);
    page  = slots[slot].next;

    if (page->next != page)
    {
        if (shift < exact_shift)
        {
            bitmap = (uintptr_t *)hcore_slab_get_page_addr(pool, page,
                                                           pagesize_shift);

            map_n = (pagesize >> shift) / (8 * sizeof(uintptr_t));

            for (i = 0; i < map_n; i++)
            {
                if (bitmap[i] != HCORE_SLAB_BUSY)
                {
                    for (m = 1, bit_n = 0; m; m <<= 1, bit_n++)
                    {
                        if (bitmap[i] & m) continue;

                        bitmap[i] |= m; // set bit

                        hcore_uint_t size = (i * 8 * sizeof(uintptr_t) + i)
                                            << shift;

                        p = (uintptr_t)bitmap + size;

                        if (bitmap[i] == HCORE_SLAB_BUSY)
                        {
                            for (i = i + 1; i < map_n; i++)
                            {
                                if (bitmap[i] != HCORE_SLAB_BUSY)
                                {
                                    goto done;
                                }
                            }

                            hcore_slab_remove_full_page(page, HCORE_SLAB_SMALL);
                        }

                        goto done;
                    }
                }
            }
        }
        else if (shift == exact_shift)
        {
            for (m = 1, bit_n = 0; m; m << 1, bit_n++)
            {
                if (page->slab & m) continue;

                page->slab |= m; // set bit

                if (page->slab == HCORE_SLAB_BUSY)
                {
                    hcore_slab_remove_full_page(page, HCORE_SLAB_EXACT);
                }

                goto done;
            }
        }
        else // shift > exact_shift
        {
            uintptr_t mask;

            /*
             * count amount of slabs and represent it with mask (each bit
             * equal to 1 slab) */
            mask = ((uintptr_t)1 << (pagesize >> shift)) - 1;
            mask <<= HCORE_SLAB_MAP_SHIFT;

            for (m = (uintptr_t)1 << HCORE_SLAB_MAP_SHIFT, bit_n = 0; m & mask;
                 m <<= 1, bit_n++)
            {
                if (page->slab & m) continue;

                page->slab |= m; // set bit

                if (page->slab & HCORE_SLAB_MAP_MASK == mask)
                {
                    hcore_slab_remove_full_page(page, HCORE_SLAB_BIG);
                }

                p = hcore_slab_get_page_addr(pool, page, pagesize_shift);


                goto done;
            }
        }
    }
    // else the slot is empty

    page = hcore_slab_alloc_pages(pool, 1);

    if (page)
    {
        if (shift < exact_shift)
        {
            hcore_uint_t n;

            bitmap = (uintptr_t *)hcore_slab_get_page_addr(pool, page,
                                                           pagesize_shift);

            // count amount of slabs of as maps
            n = (pagesize >> shift) /* slabs */
                / ((1 << shift) * 8 /* bit space of each slab */);

            if (n == 0) n = 1;

            /* "n" elements for bitmap, plus one requested */

            for (i = 0;
                 i < (n + 1) / (8 * sizeof(uintptr_t) /* bits of a map(, and 1 bit represent 1 slab) */); i++)
            {
                /*
                 * map self used size */
                bitmap[i] = HCORE_SLAB_BUSY;
            }

            /*
             * count maps of remainder */
            m = ((uintptr_t)1 << ((n + 1) % (8 * sizeof(uintptr_t)))) - 1;
            bitmap[i] = m;

            map_n = (pagesize >> shift) / (8 * sizeof(uintptr_t));

            for (i = i + 1; i < map_n; i++)
            {
                bitmap[i] = 0;
            }

            page->slab = shift;
            page->next = &slots[slot];
            hcore_slab_set_page_prev(page, &slots[slot], HCORE_SLAB_SMALL);

            slots[slot].next = page;

            p = hcore_slab_get_page_addr(pool, page, pagesize_shift)
                + (n << shift);

            goto done;
        }
        else if (shift == exact_shift)
        {
            page->slab = 1; // fitst slab
            page->next = &slots[slot];
            hcore_slab_set_page_prev(page, &slots[slot], HCORE_SLAB_EXACT);

            slots[slot].next = page;

            p = hcore_slab_get_page_addr(pool, page, pagesize_shift);

            goto done;
        }
        else // shift > exact_shift
        {
            page->slab = ((uintptr_t)1 << HCORE_SLAB_MAP_SHIFT) | shift;
            page->next = &slots[slot];
            hcore_slab_set_page_prev(page, &slots[slot], HCORE_SLAB_BIG);

            slots[slot].next = page;

            p = hcore_slab_get_page_addr(pool, page, pagesize_shift);

            goto done;
        }
    }

    p = 0;

done:
    return (void *)p;
}

static void
hcore_slab_free_pages(hcore_slab_pool_t *pool, hcore_slab_page_t *page,
                      hcore_int_t page_n)
{
    hcore_slab_page_t *prev, *join;

    pool->pfree += page_n;

    // TODO: why update the slab?
    page->slab = page_n--;

    if (page_n)
    {
        hcore_memzero(&page[1], page_n * sizeof(hcore_slab_page_t));
    }

    if (page->next)
    {
        // update pointor
        prev             = hcore_slab_get_page_prev(page);
        prev->next       = page->next;
        page->next->prev = page->prev;
    }

    join = page + page->slab; // next page

    if (join < pool->last /* isn't at end */)
    {
        /*
         * join next page to the free page if type of next page is
         * 'HCORE_SLAB_PAGE' */

        if (hcore_slab_get_page_type(join) == HCORE_SLAB_PAGE)
        {
            if (join->next != NULL)
            {
                page_n += join->slab;
                page->slab += join->slab;

                prev             = hcore_slab_get_page_prev(join);
                prev->next       = join->next;
                join->next->prev = join->prev;

                hcore_slab_remove_full_page(join, HCORE_SLAB_PAGE);
                page->slab = HCORE_SLAB_PAGE_FREE;
            }
        }
    }

    if (page > pool->pages /* isn't at start */)
    {
        /*
         * join the free page to prev page if type of prev page is
         * 'HCORE_SLAB_PAGE' */

        join = page - 1;

        if (hcore_slab_get_page_type(join) == HCORE_SLAB_PAGE)
        {
            if (join->slab == HCORE_SLAB_PAGE_FREE)
            {
                join = hcore_slab_get_page_prev(join);
            }

            if (join->next != NULL)
            {
                page_n += join->slab;
                join->slab += page->slab;

                hcore_slab_remove_full_page(join, HCORE_SLAB_PAGE);
                join->slab = HCORE_SLAB_PAGE_FREE;

                page = join;
            }
        }
    }

    if (page_n) page[page_n].prev = (uintptr_t)page;

    page->prev = (uintptr_t)&pool->free;
    page->next = pool->free.next;

    page->next->prev = (uintptr_t)page;

    pool->free.next = page;
}

static hcore_slab_page_t *
hcore_slab_alloc_pages(hcore_slab_pool_t *pool, hcore_uint_t alloc_page_n)
{
    hcore_slab_page_t *page, *p;

    for (page = pool->free.next; page != &pool->free; page = page->next)
    {
        if (alloc_page_n <= page->slab)
        {
            if (alloc_page_n < page->slab)
            {
                // point to head of the pages
                page[page->slab - 1].prev = (uintptr_t)&page[alloc_page_n];

                // update slab number
                page[alloc_page_n].slab = page->slab - alloc_page_n;

                // update pointor
                page[alloc_page_n].next = page->next;
                page[alloc_page_n].prev = page->prev;
                p                       = (hcore_slab_page_t *)page->prev;
                p->next                 = &page[alloc_page_n];
                page->next->prev        = (uintptr_t)&page[alloc_page_n];
            }
            else // alloc_page_n == page->slab
            {
                p                = (hcore_slab_page_t *)page->prev;
                p->next          = page->next;
                page->next->prev = page->prev;
            }

            // update the head page of allocated pages
            page->slab =
                alloc_page_n
                | HCORE_SLAB_PAGE_START /* flag start of allocated pages */
                ;
            page->next = NULL;
            hcore_slab_set_page_prev(page, 0, HCORE_SLAB_PAGE);

            pool->pfree -= alloc_page_n;

            if (--alloc_page_n == 0)
            {
                // only allocate one page
                return page;
            }

            // update allocated pages exclude head page
            for (p = page + 1; alloc_page_n; alloc_page_n--)
            {
                p->slab = HCORE_SLAB_PAGE_BUSY;
                p->next = NULL;
                hcore_slab_set_page_prev(page, 0, HCORE_SLAB_PAGE);
                p++;
            }

            return page;
        }
    }

    return NULL;
}


void *
hcore_shpool_alloc(hcore_shpool_t *shpool, size_t size)
{
    void *p;

    hcore_assert(shpool && size);

    if (shpool == NULL && size == 0) return NULL;

    hcore_shmtx_lock(&shpool->mutex);
    {
        p = hcore_slab_alloc_locked(shpool->sp, size);
    }
    hcore_shmtx_unlock(&shpool->mutex);

    return p;
}

void *
hcore_shpool_alloc_locked(hcore_shpool_t *shpool, size_t size)
{
    hcore_assert(shpool && size);

    return hcore_slab_alloc_locked(shpool->sp, size);
}

void *
hcore_shpool_calloc(hcore_shpool_t *shpool, size_t size)
{
    void *p;

    p = hcore_shpool_alloc(shpool, size);
    if (p)
    {
        hcore_memzero(p, size);
    }

    return p;
}

void
hcore_shpool_free(hcore_shpool_t *shpool, void *p)
{
    hcore_assert(shpool && p);

    if (shpool == NULL && p == NULL) return;

    hcore_shmtx_lock(&shpool->mutex);
    {
        hcore_slab_free_locked(shpool->sp, p);
    }
    hcore_shmtx_unlock(&shpool->mutex);
}

void
hcore_shpool_free_locked(hcore_shpool_t *shpool, void *p)
{
    hcore_assert(shpool && p);
    hcore_slab_free_locked(shpool->sp, p);
}

void
hcore_destroy_shpool(hcore_shpool_t *shpool)
{
    hcore_assert(shpool);

    if (shpool == NULL) return;

    if (munmap(shpool->addr, shpool->size) == -1)
    {
        hcore_log_error(HCORE_LOG_ALERT, shpool->log, errno,
                        "munmap(%p, %uz) failed", shpool->addr, shpool->size);
    }

    if (shpool->name && shpool->create == 0)
    {
        if (shm_unlink(shpool->name) == -1)
        {
            hcore_log_error(HCORE_LOG_ALERT, shpool->log, errno,
                            "shm_unlink(%s) failed", shpool->name);
        }
    }

    hcore_free(shpool);
}

#ifdef _HCORE_DEBUG
void
hcore_shpool_dump(hcore_shpool_t *shpool)
{
    hcore_slab_pool_t *sp = shpool->sp;

    hcore_log_debug(shpool->log, 0,
                    "log: %p, addr: %p, sp: %p, size: %uz, name: %s%N"
                    "min_shift: %uz, min_size: %uz, pages: %p, last: %p%N"
                    "pfree: %ud%N"
                    "addr: %p, start: %p, end: %p",
                    shpool->log, shpool->addr, shpool->sp, shpool->size,
                    shpool->name ? shpool->name : "anonymity", sp->min_shift,
                    sp->min_size, sp->pages, sp->last, sp->pfree, sp->addr,
                    sp->start, sp->end);
}
#endif