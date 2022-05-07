/**
 * @file hcore_array.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief 动态数组结构
 * @version 0.1
 * @date 2021-09-26
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#ifndef _HCORE_ARRAY_H_INCLUDED_
#define _HCORE_ARRAY_H_INCLUDED_

#include <hcore_constant.h>
#include <hcore_pool.h>
#include <hcore_types.h>

typedef struct
{
    void         *elts;
    hcore_uint_t  nelts;
    size_t        size;
    hcore_uint_t  nalloc;
    hcore_pool_t *pool;
} hcore_array_t;

/**
 * @brief 创建一个动态数组结构
 * @note
 * @param  *p: 内存池
 * @param  n: 预创建个数
 * @param  size: 元素的大小
 * @retval
 * 成功返回：数组结构
 * 失败返回：NULL
 */
hcore_array_t *hcore_array_create(hcore_pool_t *p, hcore_uint_t n, size_t size);

/**
 * @brief  销毁一个动态数组结构
 * @note
 * @param  *a: 数组结构
 * @retval None
 */
void hcore_array_destroy(hcore_array_t *a);

/**
 * @brief
 * 在数组中增加一个新的元素，该元素作为返回值返回，对返回值进行赋值以初始化元素的内容
 * @note
 * @param  *a:
 * @retval
 * 成功返回：新增元素
 * 失败返回：NULL
 */
void *hcore_array_push(hcore_array_t *a);

/**
 * @brief
 * 在数组中增加'n'个新的元素，并以数组的形式作为返回值返回，对返回值进行赋值以初始化元素内容
 * @note
 * @param  *a: 数组结构
 * @param  n: 要增加的元素个数
 * @retval
 * 成功返回：新增的n个元素
 * 失败返回：NULL
 */
void *hcore_array_push_n(hcore_array_t *a, hcore_uint_t n);

/**
 * @brief  初始化一个动态数组结构
 * @note
 * @param  *array: 数组结构
 * @param  *pool: 内存池
 * @param  n: 预创建元素个数
 * @param  size: 元素的大小
 * @retval
 */
static inline hcore_int_t
hcore_array_init(hcore_array_t *array, hcore_pool_t *pool, hcore_uint_t n,
                 size_t size)
{
    /*
     * set "array->nelts" before "array->elts", otherwise MSVC thinks
     * that "array->nelts" may be used without having been initialized
     */
    array->nelts  = 0;
    array->size   = size;
    array->nalloc = n;
    array->pool   = pool;

    array->elts = hcore_palloc(pool, n * size);
    if (array->elts == NULL) { return HCORE_ERROR; }

    return HCORE_OK;
}

#endif // !_HCORE_ARRAY_H_INCLUDED_
