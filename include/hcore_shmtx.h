/**
 * @file hcore_shmtx.h
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

#ifndef _HCORE_SHMTX_H_INCLUDED_
#define _HCORE_SHMTX_H_INCLUDED_

#include <hcore_types.h>

/**
 * @brief the space must in shared memory
 */
typedef struct
{
    hcore_atomic_t lock;
} hcore_shmtx_sh_t;

typedef struct
{
    hcore_atomic_t *lock; // point to hcore_shmtx_sh_t.lock
    hcore_uint_t    spin;
} hcore_shmtx_t;

/**
 * @brief initialize a shared mutex
 * 
 * @param mtx shared mutex
 * @param addr shared memory address
 * @return hcore_int_t HCORE_OK on success, HCORE_ERROR on failure
 */
hcore_int_t  hcore_shmtx_init(hcore_shmtx_t *mtx, hcore_shmtx_sh_t *addr);
/**
 * @brief try to lock a shared mutex
 * 
 * @param mtx shared mutex
 * @return hcore_uint_t 1 on success, 0 on failure
 */
hcore_uint_t hcore_shmtx_trylock(hcore_shmtx_t *mtx);
/**
 * @brief lock a shared mutex
 * 
 * @param mtx shared mutex
 */
void         hcore_shmtx_lock(hcore_shmtx_t *mtx);
/**
 * @brief unlock a shared mutex
 * 
 * @param mtx shared mutex
 */
void         hcore_shmtx_unlock(hcore_shmtx_t *mtx);
/**
 * @brief force unlock a shared mutex
 * 
 * @param mtx shared mutex
 * @param pid process id
 * @return hcore_uint_t 1 on success, 0 on failure
 */
hcore_uint_t hcore_shmtx_force_unlock(hcore_shmtx_t *mtx, hcore_pid_t pid);

#endif //!_HCORE_SHMTX_H_INCLUDED_