/**
 * @file hcore_event.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief 提供事件接口
 * @version 0.1
 * @date 2021-09-26
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#ifndef _HCORE_EVENT_H_INCLUDED_
#define _HCORE_EVENT_H_INCLUDED_

#include <hcore_array.h>
#include <hcore_constant.h>
#include <hcore_log.h>
#include <hcore_rbtree.h>
#include <hcore_types.h>

#include <signal.h>
#include <sys/select.h>
#include <sys/time.h>

/* type of event */

#define HCORE_EVENT_READ  0x01
#define HCORE_EVENT_WRITE 0x02

/* flag of event */

/**
 * @brief 该标志仅在删除事件时使用，调用者主动表明在调用"删除事件"后，
 ** 描述符也会立刻被删除。设立此标志的原因是，不同的事件需要对此有不同的处理。
 */
#define HCORE_CLOSE_EVENT 0x01

typedef int (*sde_event_get_debug_id_pt)(void *data);
typedef struct hcore_event_s hcore_event_t;

typedef void (*hcore_event_handler_pt)(struct hcore_event_s *event);

struct hcore_event_s
{
    hcore_event_handler_pt handler; // callback function on event was triggered
    hcore_log_t           *log;     // log
    void                  *data;    // data of 'handler'

    hcore_rbtree_node_t timer; // timer of event

    /* for debug */
    sde_event_get_debug_id_pt get_id;

    /* status */
    hcore_uint_t error      : 1; // encounter a error
    hcore_uint_t ready      : 1; // ready for read or write
    hcore_uint_t active     : 1; // already was added to event
    hcore_uint_t closed     : 1; // event was closed
    hcore_uint_t write      : 1; // writing event
    hcore_uint_t timer_set  : 1; // timer already was set
    hcore_uint_t timeout    : 1; // timeout for timer
    hcore_uint_t eof        : 1;
    hcore_uint_t cancelable : 1; // don't wait to close at exiting

    /* private status */
    hcore_uint_t deleted : 1;
};

/**
 * @brief  诊断事件的 'flag' 的值是否有效
 * @note
 * @retval
 */
#define hcore_event_flag_assert(flag) \
    hcore_assert((flag) & (HCORE_EVENT_READ | HCORE_EVENT_WRITE))

#endif // !_HCORE_EVENT_H_INCLUDED_
