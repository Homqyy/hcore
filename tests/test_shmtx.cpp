extern "C"
{
#include <hcore_log.h>
#include <hcore_shmtx.h>
#include <hcore_shpool.h>
}

#include <gtest/gtest.h>

class ShmtxTest : public ::testing::Test {
  protected:
    void
    SetUp() override
    {
        hcore_open_log(&fLog, (char *)"@STDOUT", HCORE_LOG_DEBUG);
        fShpoolAnonymity = hcore_create_shpool(&fLog, NULL, 1024);
        fSharedLock      = (hcore_shmtx_sh_t *)hcore_shpool_alloc(
                 fShpoolAnonymity, sizeof(hcore_shmtx_sh_t));
        hcore_shmtx_init(&fMutex, fSharedLock);
    }

    void
    TearDown() override
    {
        hcore_destroy_shpool(fShpoolAnonymity);
        hcore_destroy_log(&fLog);
    }

    hcore_shpool_t   *fShpoolAnonymity;
    hcore_log_t       fLog;
    hcore_shmtx_sh_t *fSharedLock;
    hcore_shmtx_t     fMutex;
};

TEST_F(ShmtxTest, init)
{
#ifdef _HCORE_DEBUG
    EXPECT_DEBUG_DEATH(hcore_shmtx_init(NULL, NULL),
                       "hcore_shmtx_init: Assertion");
#else
    EXPECT_EQ(hcore_shmtx_init(NULL, NULL), HCORE_ERROR);
#endif

    hcore_shmtx_t     mtx;
    hcore_shmtx_sh_t *shlock;

    shlock = (hcore_shmtx_sh_t *)hcore_shpool_alloc(fShpoolAnonymity,
                                                    sizeof(hcore_shmtx_sh_t));

    EXPECT_EQ(hcore_shmtx_init(&mtx, shlock), HCORE_OK);
}

TEST_F(ShmtxTest, lockInSingleProcess)
{
    hcore_shmtx_lock(&fMutex);

    for (hcore_uint_t i = 0; i < 5; i++)
    {
        EXPECT_FALSE(hcore_shmtx_trylock(&fMutex));
    }

    hcore_shmtx_unlock(&fMutex);

    EXPECT_TRUE(hcore_shmtx_trylock(&fMutex));

    hcore_shmtx_unlock(&fMutex);
}

TEST_F(ShmtxTest, lockInMultiprocess)
{
    hcore_pid_t pid = fork();
    if (pid == 0)
    {
        hcore_shmtx_lock(&fMutex);
        usleep(20000);
        hcore_shmtx_unlock(&fMutex);
        usleep(40000);
        EXPECT_FALSE(hcore_shmtx_trylock(&fMutex));
        usleep(60000);
        EXPECT_TRUE(hcore_shmtx_trylock(&fMutex));

        return;
    }

    usleep(10000);
    EXPECT_FALSE(hcore_shmtx_trylock(&fMutex));
    usleep(30000);
    EXPECT_TRUE(hcore_shmtx_trylock(&fMutex));
    usleep(50000);
    hcore_shmtx_unlock(&fMutex);
    usleep(70000);
    EXPECT_FALSE(hcore_shmtx_trylock(&fMutex));
    EXPECT_TRUE(hcore_shmtx_force_unlock(&fMutex, pid));
    EXPECT_TRUE(hcore_shmtx_trylock(&fMutex));
}