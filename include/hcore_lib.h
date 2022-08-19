/**
 * @file hcore_lib.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief
 * * <stdlib.h>的包裹函数，和一些基础接口。任何用到堆的接口应当统一用此包裹函数取代，在打开调试版本后，可以进行堆的追踪
 * @version 0.1
 * @date 2021-09-26
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#ifndef _HCORE_LIB_H_INCLUDED_
#define _HCORE_LIB_H_INCLUDED_

#include <stddef.h>

#define hcore_unlink(pathname) unlink((const char *)pathname)

void *hcore_malloc(size_t size);
void *hcore_calloc(size_t count, size_t size);
void *hcore_realloc(void *ptr, size_t size);
void  hcore_free(void *ptr);

#endif // !_HCORE_LIB_H_INCLUDED_
