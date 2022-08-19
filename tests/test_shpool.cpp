extern "C"
{
#include <hcore_base.h>
#include <hcore_list.h>
#include <hcore_log.h>
#include <hcore_shpool.h>
#include <hcore_string.h>
}

#include <gtest/gtest.h>

class ShpoolTest : public ::testing::Test {
  protected:
    void
    SetUp() override
    {
        hcore_open_log(&log, (char *)"@STDOUT", HCORE_LOG_DEBUG);
        shpoolWithName  = hcore_create_shpool(&log, "ShpoolTest", 1024);
        shpoolAnonymity = hcore_create_shpool(&log, NULL, 1024);
        bigShpoolWithName =
            hcore_create_shpool(&log, "BigShpoolTest", 1024 * 1024);
        bigShpoolAnonymity = hcore_create_shpool(&log, NULL, 1024 * 1024);
    }

    void
    TearDown() override
    {
        hcore_destroy_shpool(shpoolWithName);
        hcore_destroy_shpool(bigShpoolWithName);
        hcore_destroy_shpool(shpoolAnonymity);
        hcore_destroy_shpool(bigShpoolAnonymity);
        hcore_destroy_log(&log);
    }

    hcore_shpool_t *shpoolWithName;
    hcore_shpool_t *shpoolAnonymity;
    hcore_shpool_t *bigShpoolWithName;
    hcore_shpool_t *bigShpoolAnonymity;
    hcore_log_t     log;
};

static void
testSlabPoolValueAtInit(hcore_shpool_t *shpool)
{
    hcore_slab_page_t *slots, *page;
    hcore_slab_pool_t *sp;
    hcore_uint_t       remainder;
    hcore_uint_t       page_n, i;
    hcore_int_t        pagesize, pagesize_shift;

    pagesize       = hcore_getpagesize();
    pagesize_shift = hcore_getpagesize_shift();
    sp             = shpool->sp;

    ASSERT_TRUE(sp);
    EXPECT_EQ(sp->addr, shpool->addr);
    EXPECT_EQ(sp->end, shpool->addr + shpool->size);
    EXPECT_LE(sp->addr, sp->start);
    EXPECT_LE(sp->start, sp->end);

    remainder = (uintptr_t)sp->start % (uintptr_t)pagesize;
    EXPECT_EQ(remainder, 0);

    page_n = (sp->end - sp->start) / pagesize;
    ASSERT_TRUE(page_n);
    EXPECT_EQ(sp->pages + page_n, sp->last);
    EXPECT_EQ(sp->pfree, page_n);

    slots = (hcore_slab_page_t *)(sp + 1);

    for (i = 0; i < pagesize_shift - sp->min_shift; i++)
    {
        EXPECT_EQ(slots[i].slab, 0);
        EXPECT_EQ(slots[i].next, &slots[i]);
        EXPECT_EQ(slots[i].prev, 0);
    }

    EXPECT_EQ(sp->free.slab, 0);
    EXPECT_EQ(sp->free.next, sp->pages);
    EXPECT_EQ(sp->free.prev, 0);

    page = sp->pages;
    EXPECT_EQ(page->slab, page_n);
    EXPECT_EQ(page->next, &sp->free);
    EXPECT_EQ(page->prev, (uintptr_t)&sp->free);
}

static void
TestSlabPoolAfterAllocSmall(hcore_shpool_t *shpool, size_t alloced_size,
                            hcore_uint_t alloced_pages)
{
    hcore_slab_pool_t *sp = shpool->sp;
    hcore_slab_page_t *slots, *page;
    hcore_int_t        pagesize, size, exact_shift, exact_size, shift;
    hcore_uint_t       slot, page_n;

    pagesize = hcore_getpagesize();

    if (sp->min_size < alloced_size)
    {
        shift = 1; // include remainder
        for (size_t s = alloced_size - 1
             /* To avoid to exceed need mininum size when no remainder */
             ;
             s >>= 1; shift++)
        {
            // nothing
        }

        slot = shift - sp->min_shift;
    }
    else
    {
        shift = sp->min_shift;
        slot  = 0;
    }

    page_n = (sp->end - sp->start) / pagesize;

    EXPECT_EQ(sp->pfree, page_n - alloced_pages);

    slots = (hcore_slab_page_t *)(shpool->sp + 1);
    page  = slots[slot].next;

    EXPECT_NE(page->next, page);
}

TEST(shpoolTest, create)
{
    hcore_log_t     log = {.fd = -1};
    hcore_shpool_t *shpool;
    hcore_int_t     pagesize = hcore_getpagesize();

    ASSERT_EQ(hcore_open_log(&log, (char *)"@STDOUT", HCORE_LOG_DEBUG),
              HCORE_OK);

    // Null
#ifdef _HCORE_DEBUG
    EXPECT_DEBUG_DEATH(hcore_create_shpool(NULL, NULL, 0),
                       "hcore_create_shpool: Assertion");
#else
    EXPECT_FALSE(hcore_create_shpool(NULL, NULL, 0));
#endif

    // size < minimum
    shpool = hcore_create_shpool(&log, NULL, 1024);
    ASSERT_TRUE(shpool);
    EXPECT_EQ(shpool->log, &log);
    EXPECT_FALSE(shpool->name);
    EXPECT_TRUE(shpool->addr);
    EXPECT_EQ(shpool->size, pagesize << 3);
    EXPECT_TRUE(shpool->create);

    testSlabPoolValueAtInit(shpool);

    hcore_destroy_shpool(shpool);

    // size > minimum
    shpool = hcore_create_shpool(&log, NULL, 55555);
    ASSERT_TRUE(shpool);
    EXPECT_EQ(shpool->log, &log);
    EXPECT_FALSE(shpool->name);
    EXPECT_TRUE(shpool->addr);
    EXPECT_EQ(shpool->size, 55555);
    EXPECT_TRUE(shpool->create);

    testSlabPoolValueAtInit(shpool);

    hcore_destroy_shpool(shpool);

    // provide name
    shpool = hcore_create_shpool(&log, "testCreatePool", 55555);
    ASSERT_TRUE(shpool);
    EXPECT_EQ(shpool->log, &log);
    EXPECT_EQ(shpool->name, "testCreatePool");
    EXPECT_TRUE(shpool->addr);
    EXPECT_EQ(shpool->size, 55555);
    EXPECT_TRUE(shpool->create);

    testSlabPoolValueAtInit(shpool);

    hcore_destroy_shpool(shpool);

    hcore_destroy_log(&log);
}

TEST_F(ShpoolTest, allocateSmall)
{
    hcore_uchar_t *p;
    size_t         size[] = {
                shpoolAnonymity->sp->min_size - 1,
                shpoolAnonymity->sp->min_size + 1,
    };

    hcore_uint_t n = HCORE_ARRAY_NUM(size);

    for (hcore_uint_t i = 0; i < n; i++)
    {
        p = (hcore_uchar_t *)hcore_shpool_alloc(shpoolAnonymity, size[i]);
        EXPECT_TRUE(p);
        TestSlabPoolAfterAllocSmall(shpoolAnonymity, size[i], i + 1);

        p = (hcore_uchar_t *)hcore_shpool_alloc(shpoolWithName, size[i]);
        EXPECT_TRUE(p);
        TestSlabPoolAfterAllocSmall(shpoolWithName, size[i], i + 1);

        p = (hcore_uchar_t *)hcore_shpool_alloc(bigShpoolAnonymity, size[i]);
        EXPECT_TRUE(p);
        TestSlabPoolAfterAllocSmall(bigShpoolAnonymity, size[i], i + 1);

        p = (hcore_uchar_t *)hcore_shpool_alloc(bigShpoolWithName, size[i]);
        EXPECT_TRUE(p);
        TestSlabPoolAfterAllocSmall(bigShpoolWithName, size[i], i + 1);
    }

    for (hcore_uint_t i = 0; i < n; i++)
    {
        p = (hcore_uchar_t *)hcore_shpool_alloc(shpoolAnonymity, size[i]);
        EXPECT_TRUE(p);
        TestSlabPoolAfterAllocSmall(shpoolAnonymity, size[i], n);

        p = (hcore_uchar_t *)hcore_shpool_alloc(shpoolWithName, size[i]);
        EXPECT_TRUE(p);
        TestSlabPoolAfterAllocSmall(shpoolWithName, size[i], n);

        p = (hcore_uchar_t *)hcore_shpool_alloc(bigShpoolAnonymity, size[i]);
        EXPECT_TRUE(p);
        TestSlabPoolAfterAllocSmall(bigShpoolAnonymity, size[i], n);

        p = (hcore_uchar_t *)hcore_shpool_alloc(bigShpoolWithName, size[i]);
        EXPECT_TRUE(p);
        TestSlabPoolAfterAllocSmall(bigShpoolWithName, size[i], n);
    }
}

TEST_F(ShpoolTest, allocateSmallToFull) { GTEST_SKIP() << "expect to fill"; }

TEST_F(ShpoolTest, allocateSmallAdLocked)
{
    hcore_uchar_t *p;
    size_t         size[] = {
                shpoolAnonymity->sp->min_size - 1,
                shpoolAnonymity->sp->min_size + 1,
    };

    hcore_uint_t n = HCORE_ARRAY_NUM(size);

    for (hcore_uint_t i = 0; i < n; i++)
    {
        hcore_shpool_lock(shpoolAnonymity);
        {
            p = (hcore_uchar_t *)hcore_shpool_alloc_locked(shpoolAnonymity,
                                                           size[i]);
            EXPECT_TRUE(p);
            TestSlabPoolAfterAllocSmall(shpoolAnonymity, size[i], i + 1);
        }
        hcore_shpool_unlock(shpoolAnonymity);

        hcore_shpool_lock(shpoolWithName);
        {
            p = (hcore_uchar_t *)hcore_shpool_alloc_locked(shpoolWithName,
                                                           size[i]);
            EXPECT_TRUE(p);
            TestSlabPoolAfterAllocSmall(shpoolWithName, size[i], i + 1);
        }
        hcore_shpool_unlock(shpoolWithName);

        hcore_shpool_lock(bigShpoolAnonymity);
        {
            p = (hcore_uchar_t *)hcore_shpool_alloc_locked(bigShpoolAnonymity,
                                                           size[i]);
            EXPECT_TRUE(p);
            TestSlabPoolAfterAllocSmall(bigShpoolAnonymity, size[i], i + 1);
        }
        hcore_shpool_unlock(bigShpoolAnonymity);

        hcore_shpool_lock(bigShpoolWithName);
        {
            p = (hcore_uchar_t *)hcore_shpool_alloc_locked(bigShpoolWithName,
                                                           size[i]);
            EXPECT_TRUE(p);
            TestSlabPoolAfterAllocSmall(bigShpoolWithName, size[i], i + 1);
        }
        hcore_shpool_unlock(bigShpoolWithName);
    }

    for (hcore_uint_t i = 0; i < n; i++)
    {
        hcore_shpool_lock(shpoolAnonymity);
        {
            p = (hcore_uchar_t *)hcore_shpool_alloc_locked(shpoolAnonymity,
                                                           size[i]);
            EXPECT_TRUE(p);
            TestSlabPoolAfterAllocSmall(shpoolAnonymity, size[i], n);
        }
        hcore_shpool_unlock(shpoolAnonymity);

        hcore_shpool_lock(shpoolWithName);
        {
            p = (hcore_uchar_t *)hcore_shpool_alloc_locked(shpoolWithName,
                                                           size[i]);
            EXPECT_TRUE(p);
            TestSlabPoolAfterAllocSmall(shpoolWithName, size[i], n);
        }
        hcore_shpool_unlock(shpoolWithName);

        hcore_shpool_lock(bigShpoolAnonymity);
        {
            p = (hcore_uchar_t *)hcore_shpool_alloc_locked(bigShpoolAnonymity,
                                                           size[i]);
            EXPECT_TRUE(p);
            TestSlabPoolAfterAllocSmall(bigShpoolAnonymity, size[i], n);
        }
        hcore_shpool_unlock(bigShpoolAnonymity);

        hcore_shpool_lock(bigShpoolWithName);
        {
            p = (hcore_uchar_t *)hcore_shpool_alloc_locked(bigShpoolWithName,
                                                           size[i]);
            EXPECT_TRUE(p);
            TestSlabPoolAfterAllocSmall(bigShpoolWithName, size[i], n);
        }
        hcore_shpool_unlock(bigShpoolWithName);
    }
}

TEST_F(ShpoolTest, getpool)
{
    hcore_shpool_t *shpool;

#ifdef _HCORE_DEBUG
    EXPECT_DEBUG_DEATH(hcore_get_shpool(NULL, NULL),
                       "hcore_get_shpool: Assertion");
#else
    EXPECT_FALSE(hcore_get_shpool(NULL, NULL));
#endif

    shpool = hcore_get_shpool(&log, shpoolWithName->name);
    EXPECT_TRUE(shpool);
    EXPECT_TRUE(shpool->addr);
    EXPECT_TRUE(shpool->sp);
    EXPECT_FALSE(shpool->create);
    EXPECT_EQ(hcore_memcmp(shpoolWithName->sp->addr, shpool->sp->addr,
                           shpool->sp->end - shpool->sp->addr),
              0);
    hcore_destroy_shpool(shpool);

    shpool = hcore_get_shpool(&log, bigShpoolWithName->name);
    EXPECT_TRUE(shpool);
    EXPECT_TRUE(shpool->addr);
    EXPECT_TRUE(shpool->sp);
    EXPECT_FALSE(shpool->create);
    EXPECT_EQ(hcore_memcmp(bigShpoolWithName->sp->addr, shpool->sp->addr,
                           shpool->sp->end - shpool->sp->addr),
              0);
    hcore_destroy_shpool(shpool);
}

static int
hcore_shpool_list_compare_handler(const void *p1, const void *p2)
{
    int *n1 = *(int **)p1;
    int *n2 = *(int **)p2;

    if (*n1 > *n2)
        return 1;
    else if (*n1 < *n2)
        return -1;
    else
        return 0;
}

TEST_F(ShpoolTest, shared)
{
    hcore_bool_t  error = HCORE_BOOL_TRUE;
    hcore_pool_t *pool  = NULL;
    hcore_list_t *list  = NULL;
    pid_t         pid;
    int          *node, target;

    pool = hcore_create_custom_pool(&log, shpoolAnonymity,
                                    (hcore_pool_alloc_pt)hcore_shpool_alloc,
                                    (hcore_pool_free_pt)hcore_shpool_free,
                                    (hcore_pool_destroy_pt)hcore_destroy_pool);

    if (pool == NULL) goto error;

    hcore_log_debug(&log, 0, "create pool %p", pool);

    list = hcore_create_list(pool, hcore_shpool_list_compare_handler);
    if (list == NULL) goto error;

    hcore_log_debug(&log, 0,
                    "create list %p: pool %p, num_item: %ud, num_reserved %ud",
                    list, list->pool, list->num_item, list->num_reserved);

    hcore_shpool_dump(shpoolAnonymity);

    pid = fork();
    if (pid == 0)
    {
        hcore_log_debug(
            &log, 0,
            "list %p: pool %p, num_item: %ud, num_reserved %ud in suprocess",
            list, list->pool, list->num_item, list->num_reserved);

        node = (int *)hcore_shpool_alloc(shpoolAnonymity, sizeof(int));
        if (node == NULL) goto error;

        hcore_log_debug(&log, 0,
                        "node: %p%N"
                        "list %p: pool %p, num_item: %ud, num_reserved %ud "
                        "after alloc in suprocess",
                        node, list, list->pool, list->num_item,
                        list->num_reserved);

        hcore_shpool_dump(shpoolAnonymity);

        *node = 1024;

        hcore_log_debug(&log, 0,
                        "list %p: pool %p, num_item: %ud, num_reserved %ud "
                        "after set node in suprocess",
                        list, list->pool, list->num_item, list->num_reserved);

        hcore_shpool_lock(shpoolAnonymity);
        {
            hcore_log_debug(&log, 0,
                            "list %p: pool %p, num_item: %ud, num_reserved %ud "
                            "after locked in suprocess",
                            list, list->pool, list->num_item,
                            list->num_reserved);

            if (hcore_list_insert(list, node) != HCORE_OK)
            {
                hcore_log_debug(&log, 0, "fail to insert node");
                hcore_shpool_unlock(shpoolAnonymity);
                goto error;
            }

            hcore_log_debug(&log, 0, "success to insert node");
        }
        hcore_shpool_unlock(shpoolAnonymity);

        hcore_log_debug(&log, 0, "exit from subprocess");
        goto done;
    }

    if (waitpid(pid, NULL, 0) != pid)
    {
        hcore_log_debug(&log, 0, "wait %d subprocess failed", pid);
    }
    else
    {
        hcore_log_debug(&log, 0, "accept %d subprocess exit", pid);
    }

    target = 1024;

    node = (int *)hcore_list_search(list, &target);
    EXPECT_TRUE(node);
    EXPECT_EQ(*node, target);

done:
    error = HCORE_BOOL_FALSE;

error:
    if (error == HCORE_BOOL_TRUE) ADD_FAILURE();
}
