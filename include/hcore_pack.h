/**
 * @file hcore_pack.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief 提供一个报文结构，利用此结构可以方便的进行IPC。
 * * 其优势在于可以用一个易于开发的方式来传递进程间的数据，
 * * 可以通过接口在'buf'和'PACK'结构间自由转化
 * @version 0.1
 * @date 2021-09-26
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#ifndef _HCORE_PACK_H_INCLUDED_
#define _HCORE_PACK_H_INCLUDED_

#include <hcore_array.h>
#include <hcore_constant.h>
#include <hcore_list.h>
#include <hcore_pool.h>
#include <hcore_string.h>

typedef enum
{
    HCORE_PACK_ELEM_TYPE_INT  = 0, // Integer type
    HCORE_PACK_ELEM_TYPE_DATA = 1, // Data type
    HCORE_PACK_ELEM_TYPE_STR  = 2, // ANSI string type
} hcore_pack_elem_type_e;

enum
{
    hcore_pack_phase_init = 1,
    hcore_pack_phase_elem_name_len,
    hcore_pack_phase_elem_name,
    hcore_pack_phase_elem_type,
    hcore_pack_phase_value_num,
    hcore_pack_phase_value_data_int,
    hcore_pack_phase_value_data_value_len,
    hcore_pack_phase_value_data_value,
    hcore_pack_phase_done
};

typedef struct
{
    size_t size;
    void  *data;
} hcore_pack_elem_value_data_t; // Data

typedef struct
{
    hcore_str_t            name;   // Element name
    hcore_pack_elem_type_e type;   // Type
    hcore_array_t         *values; // values

    hcore_pool_t *pool;
} hcore_pack_elem_t;

typedef struct
{
    hcore_pool_t *pool;

    hcore_list_t *elements; // Element list

    hcore_uint_t max; // max number of elements

    hcore_uint_t ref; // referred count

    /* internal data: for buf */

    hcore_uchar_t *buf;
    size_t         total_size;

    /* internal data: state variable */

    int phase; // phase for parse

    unsigned int need_size; // size of need
    unsigned int num;       // number of elements
    unsigned int cur_num;   // number of current element

    unsigned int value_len;

    unsigned int cur_value_num; // number of current value
    unsigned int value_num;
    unsigned int elem_type;
    hcore_str_t  elem_name;

    hcore_pack_elem_t *cur_elem;
} hcore_pack_t;

/**
 * @brief  创建PACK结构
 * @note
 * @param  *log: 日志结构
 * @param  max: 最大的元素个数
 * @retval
 * 创建成功：PACK结构
 * 创建失败：NULL
 */
hcore_pack_t *hcore_pack_create(hcore_log_t *log, hcore_uint_t max);

/**
 * @brief  销毁PACK结构
 * @note
 * @param  *p: 待销毁的PACK结构
 * @retval None
 */
void hcore_pack_destroy(hcore_pack_t *p);

/**
 * @brief 解析'buffer'为PACK结构，这是一个逆向工程。
 * * 一般是由发端通过'hcore_pack_write()'从PACK转buffer并发送，
 * * 收端通过此接口来将'buffer'转回PACK。
 * @note
 * @param  *buffer: 待解析的buffer
 * @param  *last: buffer的末尾
 * @param  *pack: 用来存放解析结果的PACK结构
 * @retval
 * 添加成功：HCORE_OK
 * 添加失败：HCORE_ERROR
 */
ssize_t hcore_pack_read(const hcore_uchar_t *buffer, const hcore_uchar_t *last,
                        hcore_pack_t *pack);

/**
 * @brief 将PACK结构转位buffer，此次转化用到的空间由PACK结构中的pool提供。
 * * 一经转化，此PACK将锁定，也就是无论再添加多少，转化后的buffer仍旧是之前的值。
 * * 这是主要是考虑内存，因此一个PACK对应一个buffer是基本原则，如果想要改变PACK，应当重新创建一个PACK出来使用。
 * @note
 * @param  *pack:
 * @param  *size:
 * @retval
 * 添加成功：HCORE_OK
 * 添加失败：HCORE_ERROR
 */
hcore_uchar_t *hcore_pack_write(hcore_pack_t *pack, size_t *size);

/**
 * @brief 添加一个整形数据到'name'中
 * @note
 * @param  *p:
 * @param  *name:
 * @param  i:
 * @retval
 * 添加成功：HCORE_OK
 * 添加失败：HCORE_ERROR
 */
hcore_int_t hcore_pack_add_int(hcore_pack_t *p, const char *name,
                               unsigned int i);

/**
 * @brief  添加一个字符串到'name'中；该接口不会携带'\0'
 * @note
 * @param  *p:
 * @param  *name:
 * @param  *str:
 * @retval
 * 添加成功：HCORE_OK
 * 添加失败：HCORE_ERROR
 */
hcore_int_t hcore_pack_add_str(hcore_pack_t *p, const char *name,
                               const hcore_str_t *str);

/**
 * @brief  添加一个字符串到'name'中；该接口会携带'\0'过去
 * @note
 * @param  *p:
 * @param  *name:
 * @param  *str:
 * @retval
 * 添加成功：HCORE_OK
 * 添加失败：HCORE_ERROR
 */
hcore_int_t hcore_pack_add_str_char(hcore_pack_t *p, const char *name,
                                    const char *str);

/**
 * @brief  添加一个二进制数据到'name'中
 * @note
 * @param  *p:
 * @param  *name:
 * @param  *data:
 * @param  size:
 * @retval
 * 添加成功：HCORE_OK
 * 添加失败：HCORE_ERROR
 */
hcore_int_t hcore_pack_add_data(hcore_pack_t *p, const char *name,
                                const void *data, size_t size);

/**
 * @brief  获取'name'的首个整形数据
 * @note 除非参数有误，否则无论成功与否，'i'的值都会被初始化为 0
 * @param  *p:
 * @param  *name:
 * @param  *i:
 * @retval
 * 获取成功：HCORE_OK
 * 获取失败：HCORE_ERROR
 */
hcore_int_t hcore_pack_get_int(hcore_pack_t *p, const char *name,
                               unsigned int *i);

/**
 * @brief  获取'name'的所有整形数据，返回的数据以数组的形式存放在 'i' 中，
 * * 'num' 则存储数组的个数
 * @note 除非参数有误，否则无论成功与否，
 * * 'i'的值被初始化为 NULL，'num'被初始化为 0
 * @param  *p:
 * @param  *name:
 * @param  **i:
 * @param  *num:
 * @retval
 * 获取成功：HCORE_OK
 * 获取失败：HCORE_ERROR
 */
hcore_int_t hcore_pack_get_int_array(hcore_pack_t *p, const char *name,
                                     unsigned int **i, hcore_uint_t *num);

/**
 * @brief  获取'name'的首个字符串
 * @note 除非参数有误，否则无论成功与否，str的值都会被初始化为 { 0, "" }
 * @param  *p:
 * @param  *name:
 * @param  *str:
 * @retval
 * 获取成功：HCORE_OK
 * 获取失败：HCORE_ERROR
 */
hcore_int_t hcore_pack_get_str(hcore_pack_t *p, const char *name,
                               hcore_str_t *str);

/**
 * @brief  获取'name'的所有字符串，返回的数据以数组的形式存放在 'str' 中，
 * * 'num' 则存储数组的个数
 * @note 除非参数有误，否则无论成功与否，
 * * 'str'的值被初始化为 NULL，'num'被初始化为 0
 * @param  *p:
 * @param  *name:
 * @param  **i:
 * @param  *num:
 * @retval
 * 获取成功：HCORE_OK
 * 获取失败：HCORE_ERROR
 */
hcore_int_t hcore_pack_get_str_array(hcore_pack_t *p, const char *name,
                                     hcore_str_t **str, hcore_uint_t *num);

/**
 * @brief  获取'name'的首个二进制数据
 * @note 除非参数有误，否则无论成功与否，
 * * data的值都会被初始化为 NULL, size转化为 0
 * @param  *p:
 * @param  *name:
 * @param  *str:
 * @retval
 * 获取成功：HCORE_OK
 * 获取失败：HCORE_ERROR
 */
hcore_int_t hcore_pack_get_data(hcore_pack_t *p, const char *name, void **data,
                                size_t *size);

/**
 * @brief  获取'name'的所有二进制数据，返回的数据1以数组的形式存放在'data'中，
 * * 'num' 则存储数组的个数
 * @note 除非参数有误，否则无论成功与否，
 * * 'data'的值被初始化为 NULL，'num'被初始化为 0
 * @param  *p:
 * @param  *name:
 * @param  **data:
 * @param  *size:
 * @retval
 * 获取成功：HCORE_OK
 * 获取失败：HCORE_ERROR
 */
hcore_int_t hcore_pack_get_data_array(hcore_pack_t *p, const char *name,
                                      hcore_pack_elem_value_data_t **data,
                                      hcore_uint_t                  *num);


int hcore_pack_cmp_name(const void *p1, const void *p2);

/**
 * @brief  初始化PACK结构
 * @note
 * @param  *p:
 * @param  *pool:
 * @param  max:
 * @retval
 */
static inline hcore_int_t
hcore_pack_init(hcore_pack_t *p, hcore_pool_t *pool, hcore_uint_t max)
{
    p->pool     = pool;
    p->max      = max;
    p->elements = hcore_create_list(pool, hcore_pack_cmp_name);
    if (p->elements == NULL)
    {
        return HCORE_ERROR;
    }
    p->phase      = hcore_pack_phase_init;
    p->need_size  = sizeof(unsigned int);
    p->total_size = 4 /* number of element */;
    p->ref        = 1;

    return HCORE_OK;
}

/**
 * @brief  判断PACK结构是否已经解析完毕，该结构在“hcore_pack_read()”接口后使用；
 * * 如果在read后的此判断中返回的是
 * 假，则意味着PACK还未解析完全，需要继续推入新的数据解析。
 * @note
 * @param  =:
 * @retval
 */
#define hcore_pack_is_done(pack) (pack->phase == hcore_pack_phase_done)

#endif // !_HCORE_PACK_H_INCLUDED_
