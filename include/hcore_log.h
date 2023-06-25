/**
 * @file hcore_log.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief
 * 提供日志接口，可以方便的进行日志记录，并且有非常规范的日志格式和级别控制
 * @version 0.1
 * @date 2021-09-26
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#ifndef _HCORE_LOG_H_INCLUDED_
#define _HCORE_LOG_H_INCLUDED_

#include <hcore_types.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>

#define HCORE_LOG_FILE_STDOUT "@STDOUT"
#define HCORE_LOG_FILE_STDERR "@STDERR"

/* 日志级别 */
#define HCORE_LOG_STDERR 0
#define HCORE_LOG_EMERG  1
#define HCORE_LOG_ALERT  2
#define HCORE_LOG_CRIT   3
#define HCORE_LOG_ERR    4
#define HCORE_LOG_WARN   5
#define HCORE_LOG_NOTICE 6
#define HCORE_LOG_INFO   7
#define HCORE_LOG_DEBUG  8
#define HCORE_LOG_UNSET  ((hcore_uint_t)-1)

#define HCORE_LOG_PATH_DEFAULT      "/var/log/hcore"
#define HCORE_LOG_ERRSTR_LENGTH_MAX 2048
#define HCORE_LOG_TIME_LENGTH       sizeof("1970/09/28 12:00:00 +0000 GMT")

typedef void (*hcore_log_get_time_pt)(
    hcore_uchar_t time[HCORE_LOG_TIME_LENGTH]);
typedef struct hcore_log_s hcore_log_t;
typedef hcore_uchar_t *(*hcore_log_handler_pt)(hcore_log_t   *log,
                                               hcore_uchar_t *buf,
                                               hcore_uchar_t *last);

struct hcore_log_s
{
    hcore_uint_t log_level; // 日志级别
    int          fd;        // 日志文件的描述符
    char        *filename;  // 日志文件名
    char        *object; // 目标名称，用于记录日志（日志格式中有一个字段就是'目标'）

    /*
     * we declare "action" as "char *" because the actions are usually
     * the static strings and in the "u_char *" case we have to override
     * their types all the time
     */
    hcore_log_handler_pt handler;
    void                *data;
    char                *action;

    hcore_log_get_time_pt
        get_time; // 获取时间的回调函数，如果为空则默认调用'hcore_log_get_localtime()'

    hcore_uint_t internal : 1; // is internal log
};


void hcore_log_error_core(int level, hcore_log_t *log, hcore_err_t err,
                          const char *fmt, ...);

#define hcore_log_error(level, log, err, ...) \
    if ((log)->log_level >= level)            \
    hcore_log_error_core(level, log, err, __VA_ARGS__)

#ifdef _HCORE_DEBUG

#define hcore_log_debug(log, err, ...) \
    hcore_log_error(HCORE_LOG_DEBUG, log, err, __VA_ARGS__)

#else // _HCORE_DEBUG

#define hcore_log_debug(log, err, ...) \
    while (0)                          \
    {                                  \
    }

#endif // _HCORE_DEBUG

#define HCORE_LOG_IS_INTERNAL(log_file) ((log_file)[0] == '@')

/**
 * @brief  将日志字符串转化为对应的整型值，支持：'emerg', 'alert', 'crit',
 * 'error', 'warn', 'notice', 'info', 'debug'
 * @note
 * @param  *log_str:
 * @retval
 * 解析成功：返回对应的整形值，比如宏 "HCORE_LOG_ERROR"
 * 解析失败：HCORE_ERROR
 */
hcore_int_t hcore_log_parse_level(const char *log_str);

/**
 * @brief  将'text'输出到标准错误
 * @note
 * @param  *text:
 * @retval
 */
static inline void
hcore_write_stderr(char *text)
{
    (void)write(STDERR_FILENO, text, strlen(text));
}

/**
 * @brief  将'text'输出到标准输出
 * @note
 * @param  *text:
 * @retval
 */
static inline void
hcore_write_stdout(char *text)
{
    (void)write(STDOUT_FILENO, text, strlen(text));
}

/**
 * @brief  获取GMT的时间格式串，比如：2021/09/26 20:24:00 +0000 GMT
 * @note
 * @param  time[HCORE_LOG_TIME_LENGTH]: 用来存放时间格式串的地方
 * @retval None
 */
void hcore_log_get_time(hcore_uchar_t time[HCORE_LOG_TIME_LENGTH]);

/**
 * @brief  获取当前设备的时间格式串，比如：2021/09/26 20:24:00 +0800 CST
 * @note
 * @param  time[HCORE_LOG_TIME_LENGTH]: 用来存放时间格式串的地方
 * @retval None
 */
void hcore_log_get_localtime(hcore_uchar_t time[HCORE_LOG_TIME_LENGTH]);

/**
 * @brief  create a log
 * @note
 * @param  *pool: pool
 * @param  *log_file: log file path
 * @param  level: log level
 * @retval Upon successful return 'log', otherwise return 'NULL'
 */
hcore_log_t *hcore_create_log(hcore_pool_t *pool, char *log_file,
                              hcore_int_t level);

/**
 * @brief  destroy the log
 * @note
 * @param  *log: log
 * @retval None
 */
void hcore_destroy_log(hcore_log_t *log);

/**
 * @brief  open log by log_file
 * @note
 * @param  *log:
 * @param  *log_file:
 * @param  level:
 * @retval
 */
hcore_int_t hcore_open_log(hcore_log_t *log, char *log_file, hcore_int_t level);

#endif // !_HCORE_LOG_H_INCLUDED_
