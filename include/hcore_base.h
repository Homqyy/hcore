/**
 * @file hcore_base.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief base function
 * @version 0.1
 * @date 2021-09-26
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 * abs => absolute
 * val => value
 */

#ifndef _HCORE_BASE_H_INCLUDED_
#define _HCORE_BASE_H_INCLUDED_

#include <hcore_constant.h>
#include <hcore_types.h>

#include <sched.h>
#include <stddef.h>

/**
 * @brief  get absolute value of 'value'
 * @note
 * @param  value: value
 * @retval absolute value of 'value'
 */
#define hcore_abs(value) (((value) >= 0) ? (value) : -(value))

/**
 * @brief  get max value of 'val1' and 'val2'
 * @note
 * @param  val1: value 1
 * @param  val2: value 2
 * @retval max value of 'val1' and 'val2'
 */
#define hcore_max(val1, val2) ((val1 < val2) ? (val2) : (val1))

/**
 * @brief  get min value of 'val1' and 'val2'
 * @note
 * @param  val1: value 1
 * @param  val2: value 2
 * @retval min value of 'val1' and 'val2'
 */
#define hcore_min(val1, val2) ((val1 > val2) ? (val2) : (val1))

/**
 * @brief  align 'p' to 'a' bytes boundary
 * @note
 * @param  p: address
 * @param  a: alignment
 * @retval aligned address
 */
#define hcore_align_ptr(p, a)                               \
    (hcore_uchar_t *)(((uintptr_t)(p) + ((uintptr_t)a - 1)) \
                      & ~((uintptr_t)a - 1))

/**
 * @brief  get number of elements in array 'const_array'
 * @note
 * @param  const_array: array
 * @retval
 */
#define HCORE_ARRAY_NUM(const_array) \
    (sizeof(const_array) / sizeof(const_array[0]))

/**
 * @brief  通过数据结构的字段地址 'field' 来反向的获取数据结构 'type'
 * 的首地址，其中
 * * 'link' 是 'field' 在 'type' 中的字段名。
 *
 * @note
 * @retval type * : 返回数据结构 'type' 的首地址
 */
#define HCORE_GET_DATA_BY_FIELD(field, type, link) \
    (type *)((hcore_uchar_t *)field - offsetof(type, link))


/**
 * @brief Get pid of current process
 * @return hcore_pid_t : pid of process
 */
#define hcore_getpid() ((hcore_pid_t)getpid())

#define hcore_likely(x) __builtin_expect(!!(x), 1)

/**
 * @brief get system page size
 *
 * @return hcore_int_t : page size of system.
 */
hcore_int_t hcore_getpagesize(void);

/**
 * @brief get shift of pagesize
 *
 * @return hcore_int_t : shift of pagesize
 */
hcore_int_t hcore_getpagesize_shift(void);

/**
 * @brief Do invoke once in process. it will initialize all be required by
 * process, that is valid to accelerate to invoke other interface
 */
void hcore_process_invoke_once(void);

/**
 * @brief get max size of slab
 *
 * @return hcore_int_t : return max size of slab
 */
hcore_int_t hcore_get_slab_max_size(void);

/**
 * @brief get exact size of slab
 *
 * @return hcore_int_t : return exact size of slab
 */
hcore_int_t hcore_get_slab_exact_size(void);

/**
 * @brief get exact shift of slab
 *
 * @return hcore_int_t : return exact shift of slab
 */
hcore_int_t hcore_get_slab_exact_shift(void);

#define hcore_cpu_pause()   __asm__("pause")
#define hcore_sched_yield() sched_yield()

#define HCORE_SMP_LOCK "lock;"


#if (HCORE_HAVE_AUTOMIC_OPS)

/*
 * "cmpxchgq  r, [m]":
 *
 *     if (rax == [m]) {
 *         zf = 1;
 *         [m] = r;
 *     } else {
 *         zf = 0;
 *         rax = [m];
 *     }
 *
 *
 * The "r" is any register, %rax (%r0) - %r16.
 * The "=a" and "a" are the %rax register.
 * Although we can return result in any register, we use "a" because it is
 * used in cmpxchgq anyway.  The result is actually in %al but not in $rax,
 * however as the code is inlined gcc can test %al as well as %rax.
 *
 * The "cc" means that flags were changed.
 */
static inline hcore_atomic_uint_t
hcore_atomic_cmp_set(hcore_atomic_t *lock, hcore_atomic_uint_t old,
                     hcore_atomic_uint_t set)
{
    u_char res;

    __asm__ volatile(

        HCORE_SMP_LOCK "    cmpxchgq  %3, %1;   "
                       "    sete      %0;       "

        : "=a"(res)
        : "m"(*lock), "a"(old), "r"(set)
        : "cc", "memory");

    return res;
}


/*
 * "xaddq  r, [m]":
 *
 *     temp = [m];
 *     [m] += r;
 *     r = temp;
 *
 *
 * The "+r" is any register, %rax (%r0) - %r16.
 * The "cc" means that flags were changed.
 */

static inline hcore_atomic_int_t
hcore_atomic_fetch_add(hcore_atomic_t *value, hcore_atomic_int_t add)
{
    __asm__ volatile(

        HCORE_SMP_LOCK "    xaddq  %0, %1;   "

        : "+r"(add)
        : "m"(*value)
        : "cc", "memory");

    return add;
}

#endif // (HCORE_HAVE_AUTOMIC_OPS)


#endif // !_HCORE_BASE_H_INCLUDED_
