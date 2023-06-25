/**
 * @file hcore_shmtx.c
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
#include <hcore_debug.h>
#include <hcore_shmtx.h>

static void hcore_shmtx_wakeup(hcore_shmtx_t *mtx);

hcore_int_t
hcore_shmtx_init(hcore_shmtx_t *mtx, hcore_shmtx_sh_t *addr)
{
    hcore_assert(mtx && addr);

    if (mtx == NULL || addr == NULL) return HCORE_ERROR;

    mtx->lock = &addr->lock;

    if (mtx->spin == (hcore_uint_t)-1)
    {
        return HCORE_OK;
    }

    /* Use default spin value if not set */
    mtx->spin = 1 << 11;

    return HCORE_OK;
}

void
hcore_shmtx_lock(hcore_shmtx_t *mtx)
{
    hcore_uint_t i, n;
    hcore_pid_t  pid = hcore_getpid();

    for (;;)
    {
        // If the lock is not held by anyone, then try to acquire it.
        if (*mtx->lock == 0 && hcore_atomic_cmp_set(mtx->lock, 0, pid))
        {
            return;
        }

        // If we have not yet spun for the maximum amount of time, then spin
        // for the current amount of time.
        for (n = 1; n < mtx->spin; n <<= 1)
        {
            for (i = 0; i < n; i++)
            {
                hcore_cpu_pause();
            }

            // If the lock is not held by anyone, then try to acquire it.
            if (*mtx->lock == 0 && hcore_atomic_cmp_set(mtx->lock, 0, pid))
            {
                return;
            }
        }

        // If we have spun for the maximum amount of time, then yield the
        // remainder of our timeslice.
        hcore_sched_yield();
    }
}

hcore_uint_t
hcore_shmtx_trylock(hcore_shmtx_t *mtx)
{
    return (*mtx->lock == 0
            && hcore_atomic_cmp_set(mtx->lock, 0, hcore_getpid()));
}

void
hcore_shmtx_unlock(hcore_shmtx_t *mtx)
{
    if (hcore_atomic_cmp_set(mtx->lock, hcore_getpid(), 0))
    {
        hcore_shmtx_wakeup(mtx);
    }
}

hcore_uint_t
hcore_shmtx_force_unlock(hcore_shmtx_t *mtx, hcore_pid_t pid)
{
    if (hcore_atomic_cmp_set(mtx->lock, pid, 0))
    {
        hcore_shmtx_wakeup(mtx);
        return 1;
    }

    return 0;
}

static void
hcore_shmtx_wakeup(hcore_shmtx_t *mtx)
{
    // nothing
}