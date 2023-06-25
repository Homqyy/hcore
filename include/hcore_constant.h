/**
 * @file hcore_constant.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief 定义常量
 * @version 0.1
 * @date 2021-09-26
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#ifndef _HCORE_CONSTANT_H_INCLUDED_
#define _HCORE_CONSTANT_H_INCLUDED_

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

/* must support atomic in gcc */
#define HCORE_HAVE_GCC_ATOMIC

#ifdef HCORE_HAVE_GCC_ATOMIC

#define HCORE_HAVE_AUTOMIC_OPS 1

/* GCC 4.1 builtin atomic operations */

#define HCORE_AUTOMIC_T_LEN (sizeof("-9223372036854775808") - 1)

#endif // HCORE_HAVE_GCC_ATOMIC

#define HCORE_MACHINE_ALIGN 8

#ifndef IOV_MAX
#define IOV_MAX 16
#endif

#define HCORE_OK       0
#define HCORE_ERROR    -1
#define HCORE_AGAIN    -2
#define HCORE_DONE     -3
#define HCORE_DECLINED -4

#define HCORE_BOOL_INVALID 2
#define HCORE_BOOL_FALSE   0
#define HCORE_BOOL_TRUE    1

#define HCORE_LF   (hcore_uchar_t)'\n'
#define HCORE_CR   (hcore_uchar_t)'\r'
#define HCORE_CRLF "\r\n"

#define HCORE_LINEFEED      "\x0a"
#define HCORE_LINEFEED_SIZE 1

#define HCORE_ALIGNMENT HCORE_MACHINE_ALIGN

#define HCORE_INFINITE (0xFFFFFFFF)

#define HCORE_INT32_LEN (sizeof("-2147483648") - 1)
#define HCORE_INT64_LEN (sizeof("-9223372036854775808") - 1)
#define HCORE_INT_T_LEN HCORE_INT32_LEN

#define HCORE_MAX_INT32_VALUE   INT_MAX
#define HCORE_MAX_UINT32_VALUE  UINT_MAX
#define HCORE_MAX_INT64_VALUE   9223372036854775807LL
#define HCORE_MAX_INT_T_VALUE   HCORE_MAX_INT32_VALUE
#define HCORE_MAX_UINT_T_VALUE  HCORE_MAX_UINT32_VALUE
#define HCORE_MAX_SSIZE_T_VALUE HCORE_MAX_INT64_VALUE

#define HCORE_IOV_MAX IOV_MAX

#endif // !_HCORE_CONSTANT_H_INCLUDED_
