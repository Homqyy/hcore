/**
 * @file hcore_base.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief 基础的宏定义接口
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

#include <stddef.h>

#if !defined(HCORE_32BIT) && !defined(HCORE_64BIT)
#define HCORE_64BIT
#endif

#ifdef HCORE_32BIT
#define HCORE_MACHINE_ALIGN 4
#else
#define HCORE_MACHINE_ALIGN 8
#endif

/**
 * @brief  获取 'valud' 的绝对值
 * @note
 * @retval
 */
#define hcore_abs(value) (((value) >= 0) ? (value) : -(value))

/**
 * @brief  获取 'val1' 和 'val2' 两者间的最大值
 * @note
 * @retval
 */
#define hcore_max(val1, val2) ((val1 < val2) ? (val2) : (val1))

/**
 * @brief  获取 'val1' 和 'val2' 两者间的最小值
 * @note
 * @retval
 */
#define hcore_min(val1, val2) ((val1 > val2) ? (val2) : (val1))

/**
 * @brief  排列指针地址：按照 'a' 字节来排列指针 'p'
 * @note
 * @retval
 */
#define hcore_align_ptr(p, a) \
    (hcore_uchar_t *)(((uintptr_t)(p) + ((uintptr_t)a - 1)) & ~((uintptr_t)a - 1))

/**
 * @brief  获取常量数组的元素个数
 * @note
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
 * @retval
 */
#define HCORE_GET_DATA_BY_FIELD(field, type, link) \
    (type *)((hcore_uchar_t *)field - offsetof(type, link))

#endif // !_HCORE_BASE_H_INCLUDED_
