extern "C"
{
    #include <hcore_constant.h>
    #include <hcore_astring.h>
    #include <hcore_lib.h>
}

#include <gtest/gtest.h>

TEST(astringTest, InitNull)
{
#ifdef _HCORE_DEBUG
    EXPECT_DEBUG_DEATH(hcore_init_astring(NULL, 0, NULL, NULL), "hcore_init_astring: Assertion `astr' failed");
#else
    EXPECT_EQ(hcore_init_astring(NULL, 0, NULL, NULL), HCORE_ERROR);
#endif
}

TEST(astringTest, InitWithNull)
{
    hcore_astring_t astr;
    hcore_astring_t *pastr = &astr;

    EXPECT_EQ(hcore_init_astring(pastr, 0, NULL, NULL), HCORE_OK);
    EXPECT_FALSE(pastr->created);
    EXPECT_TRUE(pastr->data);
    EXPECT_EQ(pastr->free, free);
    EXPECT_EQ(pastr->realloc, realloc);
    EXPECT_EQ(pastr->size, HCORE_ASTRING_DEF_SIZE);
    EXPECT_EQ(pastr->error, 0);
    EXPECT_EQ(pastr->len, 0);
    EXPECT_TRUE(pastr->inited);
    hcore_destroy_astring(pastr);
}

TEST(astringTest, InitWithFullArgs)
{
    hcore_astring_t astr;
    hcore_astring_t *pastr = &astr;

    EXPECT_EQ(hcore_init_astring(pastr, 2 * 1024, hcore_realloc, hcore_free), HCORE_OK);
    EXPECT_FALSE(pastr->created);
    EXPECT_TRUE(pastr->data);
    EXPECT_EQ(pastr->free, hcore_free);
    EXPECT_EQ(pastr->realloc, hcore_realloc);
    EXPECT_EQ(pastr->size, 2 * 1024);
    EXPECT_EQ(pastr->error, 0);
    EXPECT_EQ(pastr->len, 0);
    EXPECT_TRUE(pastr->inited);
    hcore_destroy_astring(pastr);
}

TEST(astringTest, CreateWithNullArgs)
{
    hcore_astring_t *astr;

    astr = hcore_create_astring(0, NULL, NULL);

    EXPECT_TRUE(astr);
    EXPECT_TRUE(astr->created);
    EXPECT_TRUE(astr->data);
    EXPECT_EQ(astr->free, free);
    EXPECT_EQ(astr->realloc, realloc);
    EXPECT_EQ(astr->size, HCORE_ASTRING_DEF_SIZE);
    EXPECT_EQ(astr->error, 0);
    EXPECT_EQ(astr->len, 0);
    EXPECT_TRUE(astr->inited);
    hcore_destroy_astring(astr);
}

TEST(astringTest, CreateWithSpecialSize)
{
    hcore_astring_t *astr;

    // 1 B
    astr = hcore_create_astring(1, NULL, NULL);

    EXPECT_TRUE(astr);
    EXPECT_TRUE(astr->created);
    EXPECT_EQ(astr->free, free);
    EXPECT_EQ(astr->realloc, realloc);
    EXPECT_EQ(astr->size, 1);
    EXPECT_TRUE(astr->data);
    EXPECT_EQ(astr->error, 0);
    EXPECT_EQ(astr->len, 0);
    EXPECT_TRUE(astr->inited);
    hcore_destroy_astring(astr);

    // 100 MB
    astr = hcore_create_astring(100 * 1024 * 1024, NULL, NULL);

    EXPECT_TRUE(astr);
    EXPECT_TRUE(astr->created);
    EXPECT_EQ(astr->free, free);
    EXPECT_EQ(astr->realloc, realloc);
    EXPECT_EQ(astr->size, 100 *1024 * 1024);
    EXPECT_TRUE(astr->data);
    EXPECT_EQ(astr->error, 0);
    EXPECT_EQ(astr->len, 0);
    EXPECT_TRUE(astr->inited);
    hcore_destroy_astring(astr);
}

TEST(astringTest, CreateWithSpecialFunction)
{
    hcore_astring_t *astr;

    astr = hcore_create_astring(0, hcore_realloc, hcore_free);

    EXPECT_TRUE(astr);
    EXPECT_TRUE(astr->created);
    EXPECT_EQ(astr->free, hcore_free);
    EXPECT_EQ(astr->realloc, hcore_realloc);
    EXPECT_EQ(astr->size, HCORE_ASTRING_DEF_SIZE);
    EXPECT_TRUE(astr->data);
    EXPECT_EQ(astr->error, 0);
    EXPECT_EQ(astr->len, 0);
    EXPECT_TRUE(astr->inited);
    hcore_destroy_astring(astr);
}

TEST(astringTest, DestroyNull)
{
    EXPECT_DEBUG_DEATH(hcore_destroy_astring(NULL), "hcore_destroy_astring: Assertion `astr && astr->inited' failed");
}

TEST(astringTest, DestroyUninit)
{
    hcore_astring_t astr;

    EXPECT_DEBUG_DEATH(hcore_destroy_astring(&astr), "hcore_destroy_astring: Assertion `astr && astr->inited' failed") ;
}

TEST(astringTest, asnprintToNull)
{
#ifdef _HCORE_DEBUG
    EXPECT_DEBUG_DEATH(hcore_asnprintf(NULL, NULL), "hcore_asnprintf: Assertion `astr' failed");
#else
    EXPECT_EQ(hcore_asnprintf(NULL, NULL), HCORE_ERROR);
#endif
}

TEST(astringTest, asnprintfDataToDefaultObjectWithNullFormat)
{
    hcore_astring_t *astr;

    astr = hcore_create_astring(0, NULL, NULL);
    EXPECT_EQ(hcore_asnprintf(astr, NULL), HCORE_ERROR);
    hcore_destroy_astring(astr);
}

TEST(astringTest, asnprintfDataToDefaultObject)
{
    hcore_astring_t *astr;
    hcore_uint_t i;

    // do 1 action and write 88 bytes on each it
    astr = hcore_create_astring(0, NULL, NULL);
    if (astr)
    {
        char b88[88];

        for (i = 0; i < sizeof(b88); i++)
        {
            b88[i] = '1';
        }
        b88[87] = 0;

        EXPECT_EQ(hcore_asnprintf(astr, "%s%Z", b88), HCORE_OK);
        EXPECT_EQ(astr->len, 88);
        EXPECT_STREQ((const char *)astr->data, b88);
        hcore_destroy_astring(astr);
    }
    else
    {
        ADD_FAILURE();
    }

    // do 1024 action and write 1024 bytes on each it
    astr = hcore_create_astring(0, NULL, NULL);
    if (astr)
    {
        char b1024[1024];
        for (i = 0; i < sizeof(b1024); i++)
        {
            b1024[i] = '1';
        }
        b1024[1023] = 0;

        for (i = 0; i < 1024; i++)
        {
            EXPECT_EQ(hcore_asnprintf(astr, "%s%Z", b1024), HCORE_OK);
        }

        const char *p = (const char *)astr->data;
        for (i = 0; i < 1024; i++)
        {
            EXPECT_STREQ(p, b1024);
            p += sizeof(b1024);
        }

        hcore_destroy_astring(astr);
    }
    else
    {
        ADD_FAILURE();
    }

}