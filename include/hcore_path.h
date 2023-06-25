/**
 * @file hcore_path.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief
 * @version 0.1
 * @date 2021-10-09
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr: abs => absolute
 */

#ifndef _HCORE_PATH_H_INCLUDED_
#define _HCORE_PATH_H_INCLUDED_

#include <hcore_constant.h>
#include <hcore_pool.h>
#include <hcore_string.h>

/**
 * @brief test whether a path is absolute"
 *
 * @param path
 * @return hcore_bool_t
 */

#define HCORE_ABSPATH_SEP   '/'
#define hcore_is_abspath(path) path[0] == HCORE_ABSPATH_SEP

/**
 * @brief  返回一个末尾携带'\0'的完整路径；
 * * 如果'src'已经是绝对路径，则直接返回src，
 * * 反之则与'def_path'一起构成完整路径
 * @note
 * @param  *pool:
 * @param  *src:
 * @param  *def_path:
 * @retval
 */
hcore_str_t *hcore_get_full_path(hcore_pool_t *pool, const char *src,
                             hcore_str_t *def_path);

#endif // !_HCORE_PATH_H_INCLUDED_
