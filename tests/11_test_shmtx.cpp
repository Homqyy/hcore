extern "C"
{
#include <hcore_base.h>
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
        // Initialize logging.
        hcore_open_log(&fLog, (char *)"@STDOUT", HCORE_LOG_DEBUG);

        // Create an anonymous shared memory pool.
        fShpoolAnonymity = hcore_create_shpool(&fLog, NULL, 1024);

        // Allocate a shared lock.
        fSharedLock = (hcore_shmtx_sh_t *)hcore_shpool_calloc(
            fShpoolAnonymity, sizeof(hcore_shmtx_sh_t));

        // Initialize the mutex.
        hcore_shmtx_init(&fMutex, fSharedLock);
    }

    void
    TearDown() override
    {
        // Destroy the shared memory pool.
        hcore_destroy_shpool(fShpoolAnonymity);

        // Destroy the logging.
        hcore_destroy_log(&fLog);
    }

    hcore_shpool_t   *fShpoolAnonymity;
    hcore_log_t       fLog;
    hcore_shmtx_sh_t *fSharedLock;
    hcore_shmtx_t     fMutex;
};

static void
wait_next(hcore_log_t *log, const hcore_uint_t * const n, hcore_uint_t expected)
{
    hcore_log_error(HCORE_LOG_INFO, log, 0, "wait %ud and now is %ud", expected, *n);

    while (*n != expected)
    {
        usleep(1000); /* 1 ms */
    }
}

static void
notify_next(hcore_log_t *log, hcore_uint_t *n)
{
    (*n)++;

    hcore_log_error(HCORE_LOG_INFO, log, 0, "notify next to %ud", *n);
}

TEST_F(ShmtxTest, init)
{
#ifdef _HCORE_DEBUG
    EXPECT_DEBUG_DEATH(hcore_shmtx_init(NULL, NULL),
                       "hcore_shmtx_init: Assertion");
#else
    EXPECT_EQ(hcore_shmtx_init(NULL, NULL), HCORE_ERROR);
#endif

    // 1. Initialize a shared mutex with a spin value of 0.

    hcore_shmtx_t     mtx = {0};
    hcore_shmtx_sh_t *shlock;

    shlock = (hcore_shmtx_sh_t *)hcore_shpool_calloc(fShpoolAnonymity,
                                                    sizeof(hcore_shmtx_sh_t));

    EXPECT_EQ(hcore_shmtx_init(&mtx, shlock), HCORE_OK);
    EXPECT_EQ(mtx.lock, &shlock->lock);
    EXPECT_EQ(mtx.spin, 2048);

    // 2. Initialize a shared mutex with a spin value of -1.

    mtx = (hcore_shmtx_t){.spin = (hcore_uint_t)-1};

    EXPECT_EQ(hcore_shmtx_init(&mtx, shlock), HCORE_OK);
    EXPECT_EQ(mtx.lock, &shlock->lock);
    EXPECT_EQ(mtx.spin, (hcore_uint_t)-1);
}

TEST_F(ShmtxTest, trylock)
{
    // 1. Try to lock a shared mutex.

    EXPECT_TRUE(hcore_shmtx_trylock(&fMutex));
    EXPECT_EQ(fSharedLock->lock, hcore_getpid());
    hcore_shmtx_unlock(&fMutex);
    EXPECT_EQ(fSharedLock->lock, 0);

    // 2. Try to duplicate lock a shared mutex.

    EXPECT_TRUE(hcore_shmtx_trylock(&fMutex));
    EXPECT_FALSE(hcore_shmtx_trylock(&fMutex));
    hcore_shmtx_unlock(&fMutex);
    EXPECT_TRUE(hcore_shmtx_trylock(&fMutex));
    hcore_shmtx_unlock(&fMutex);

    // 3. first lock, then try to lock a shared mutex.

    hcore_shmtx_lock(&fMutex);
    EXPECT_FALSE(hcore_shmtx_trylock(&fMutex));
    hcore_shmtx_unlock(&fMutex);
}

TEST_F(ShmtxTest, lock)
{
    // 1. Lock a shared mutex.

    hcore_shmtx_lock(&fMutex);
    EXPECT_EQ(fSharedLock->lock, hcore_getpid());
    hcore_shmtx_unlock(&fMutex);
    EXPECT_EQ(fSharedLock->lock, 0);

    // 2. first lock, then try to lock a shared mutex.

    hcore_shmtx_lock(&fMutex);
    EXPECT_FALSE(hcore_shmtx_trylock(&fMutex));
    hcore_shmtx_unlock(&fMutex);

    // 3. after unlock, try to lock a shared mutex.

    EXPECT_TRUE(hcore_shmtx_trylock(&fMutex));
    hcore_shmtx_unlock(&fMutex);

    // TODO: 4. first try to lock, then lock a shared mutex
}

TEST_F(ShmtxTest, unlock)
{
    // 1. Unlock a shared mutex.

    hcore_shmtx_lock(&fMutex);
    hcore_shmtx_unlock(&fMutex);
    EXPECT_EQ(fSharedLock->lock, 0);

    // 2. Unlock a shared mutex that is not locked.

    hcore_shmtx_unlock(&fMutex);
    EXPECT_EQ(fSharedLock->lock, 0);

    // 3. Unlock a shared mutex that is locked by another process.

    hcore_pid_t pid = fork();
    hcore_uint_t *n = (hcore_uint_t *)hcore_shpool_calloc(fShpoolAnonymity,
                                                         sizeof(hcore_uint_t));

    hcore_log_error(HCORE_LOG_INFO, &fLog, 0, "pid: %d", pid);

    if (pid == 0)
    {
        // Child process.

        // 4. Lock a shared mutex.

        hcore_log_error(HCORE_LOG_INFO, &fLog, 0, "[%ud] child: lock shared mutex", *n);

        hcore_shmtx_lock(&fMutex);
        EXPECT_EQ(fSharedLock->lock, pid); 

        notify_next(&fLog, n);

        // 6. Unlock the shared mutex of "4".

        wait_next(&fLog, n, 2);

        hcore_shmtx_unlock(&fMutex);
        EXPECT_EQ(fSharedLock->lock, 0); 

        notify_next(&fLog, n);
    }
    else
    {
        // Parent process.

        // 5. Fail to unlock a shared mutex that is locked by another process.

        wait_next(&fLog, n, 1);

        hcore_log_error(HCORE_LOG_INFO, &fLog, 0, "[%ud] parent: unlock shared mutex that is locked by another process", *n);

        EXPECT_EQ(fSharedLock->lock, pid); 
        hcore_shmtx_unlock(&fMutex);
        EXPECT_EQ(fSharedLock->lock, pid);
        EXPECT_FALSE(hcore_shmtx_trylock(&fMutex));

        notify_next(&fLog, n);

        // 7. Try to lock a shared mutex to valid unlock.

        wait_next(&fLog, n, 3);

        EXPECT_TRUE(hcore_shmtx_trylock(&fMutex));
        EXPECT_EQ(fSharedLock->lock, hcore_getpid());
        hcore_shmtx_unlock(&fMutex);
        EXPECT_EQ(fSharedLock->lock, 0);

        // 8. wait for the child process to exit.

        waitpid(pid, NULL, 0);
    }
}
TEST_F(ShmtxTest, forceUnlock)
{
    // 1. Force unlock a shared mutex.

    hcore_shmtx_lock(&fMutex);
    EXPECT_EQ(fSharedLock->lock, hcore_getpid());
    EXPECT_TRUE(hcore_shmtx_force_unlock(&fMutex, hcore_getpid()));
    EXPECT_EQ(fSharedLock->lock, 0);

    // 2. Force unlock a shared mutex that is not locked.

    EXPECT_FALSE(hcore_shmtx_force_unlock(&fMutex, hcore_getpid()));
    EXPECT_EQ(fSharedLock->lock, 0);

    // 3. Force unlock a shared mutex that is locked by another process.

    hcore_pid_t pid = fork();
    hcore_uint_t *n = (hcore_uint_t *)hcore_shpool_calloc(fShpoolAnonymity,
                                                         sizeof(hcore_uint_t));

    if (pid == 0)
    {
        // Child process.

        hcore_shmtx_lock(&fMutex);
        EXPECT_EQ(fSharedLock->lock, hcore_getpid());
        notify_next(&fLog, n);
    }
    else
    {
        // Parent process.

        wait_next(&fLog, n, 1);
        EXPECT_TRUE(hcore_shmtx_force_unlock(&fMutex, pid));
        EXPECT_EQ(fSharedLock->lock, 0);

        // wait for the child process to exit.
        waitpid(pid, NULL, 0);
    }
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

    // create a shared variable to synchronize the two processes.
    hcore_uint_t *n = (hcore_uint_t *)hcore_shpool_calloc(fShpoolAnonymity,
                                                         sizeof(hcore_uint_t));

    if (pid == 0)
    {
        // Child process.

        // 1. Lock a shared mutex.

        hcore_shmtx_lock(&fMutex);
        notify_next(&fLog, n);

        // 3. unlock a shared mutex.

        wait_next(&fLog, n, 2);
        hcore_shmtx_unlock(&fMutex);
        notify_next(&fLog, n);
    }
    else
    {
        // Parent process.

        // 2. Try to lock a shared mutex, but failed.

        wait_next(&fLog, n, 1);
        EXPECT_EQ(fSharedLock->lock, pid);
        EXPECT_FALSE(hcore_shmtx_trylock(&fMutex));
        notify_next(&fLog, n);

        // 4. Try to lock a shared mutex, and succeed.

        wait_next(&fLog, n, 3);
        EXPECT_EQ(fSharedLock->lock, 0);
        EXPECT_TRUE(hcore_shmtx_trylock(&fMutex));
        EXPECT_EQ(fSharedLock->lock, hcore_getpid());
        hcore_shmtx_unlock(&fMutex);

        // 5. wait for the child process to exit.

        waitpid(pid, NULL, 0);
    }
}