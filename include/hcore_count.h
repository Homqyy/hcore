/**
 * @file hcore_count.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief 提供运算相关的接口
 * @version 0.1
 * @date 2021-09-26
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#ifndef _HCORE_COUNT_H_INCLUDED_
#define _HCORE_COUNT_H_INCLUDED_

#include <hcore_types.h>

/**
 * @brief  检查 int 乘法是否溢出，即：‘a * b’ 的结果是否溢出
 * @note
 * @param  a:
 * @param  b:
 * @retval
 */
hcore_int_t hcore_multip_is_overflow(int a, int b);

/**
 * @brief  检查 long 乘法是否溢出，即：‘a * b’ 的结果是否溢出
 * @note
 * @param  a:
 * @param  b:
 * @retval
 */
hcore_int_t hcore_multip_is_overflow_long(long a, long b);

#endif // !_HCORE_COUNT_H_INCLUDED_
