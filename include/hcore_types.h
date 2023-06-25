/**
 * @file hcore_types.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief
 * typedef定义，当hcore_core接口出现循环依赖时，需要将typedef的定义放置到此
 * @version 0.1
 * @date 2021-09-26
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#ifndef _HCORE_TYPES_H_INCLUDED_
#define _HCORE_TYPES_H_INCLUDED_

#include <hcore_constant.h>

#include <sys/types.h>

typedef unsigned int hcore_bool_t;
typedef signed int   hcore_err_t;
typedef pid_t        hcore_pid_t;
typedef unsigned int hcore_msec_t;
typedef signed int   hcore_msec_int_t;

typedef signed int           hcore_int_t;
typedef signed short         hcore_int16_t;
typedef signed int           hcore_int32_t;
typedef signed long long int hcore_int64_t;

typedef unsigned char          hcore_uchar_t;
typedef unsigned int           hcore_uint_t;
typedef unsigned short         hcore_uint16_t;
typedef unsigned int           hcore_uint32_t;
typedef unsigned long long int hcore_uint64_t;

typedef struct hcore_pool_s        hcore_pool_t;
typedef struct hcore_custom_pool_s hcore_custom_pool_t;
typedef struct hcore_chain_s       hcore_chain_t;
typedef struct hcore_buf_s         hcore_buf_t;


#if (HCORE_HAVE_AUTOMIC_OPS)

typedef long                         hcore_atomic_int_t;
typedef unsigned long                hcore_atomic_uint_t;
typedef volatile hcore_atomic_uint_t hcore_atomic_t;

#endif // HCORE_HAVE_AUTOMIC_OPS


#endif // !_HCORE_TYPES_H_INCLUDED_
