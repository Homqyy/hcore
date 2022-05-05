/**
 * @file hcore_hash.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief 提供散列接口
 * @version 0.1
 * @date 2021-09-26
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#ifndef _HCORE_HASH_H_INCLUDED_
#define _HCORE_HASH_H_INCLUDED_

#include <hcore_types.h>

#define hcore_hash(key, c) ((hcore_uint_t)key * 31 + c)

/**
 * @brief  计算一个'data'的hash值
 * @note
 * @param  *data: 待计算的数据
 * @param  len: 数据的长度
 * @retval
 */
hcore_uint_t hcore_hash_key(hcore_uchar_t *data, size_t len);

#endif // !_HCORE_HASH_H_INCLUDED_
