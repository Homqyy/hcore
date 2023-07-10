/**
 * @file hcore_log.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief Provide log interface, which can easily record logs,
 * and have very standardized log format and level control
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

#define HCORE_LOG_FILE_STDOUT ((char *)"@STDOUT")
#define HCORE_LOG_FILE_STDERR ((char *)"@STDERR")

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
    hcore_uint_t log_level; // log level
    int          fd;        // fd of log file
    char        *filename;  // log file name
    char *object; // name of the object, used for logging (there is a field in
                  // the log format that is 'target')

    /*
     * we declare "action" as "char *" because the actions are usually
     * the static strings and in the "u_char *" case we have to override
     * their types all the time */
    hcore_log_handler_pt handler;
    void                *data;
    char                *action;

    /*
     * callback function to get time,
     * if it is NULL, then call 'hcore_log_get_localtime()' by default */
    hcore_log_get_time_pt get_time;

    hcore_uint_t internal : 1; // is internal log
};


void hcore_log_error_core(int level, hcore_log_t *log, hcore_err_t err,
                          const char *fmt, ...);

/**
 * @brief record log
 *
 * @param level: log level
 * @param log: log object
 * @param err: error code
 * @param ...: format string and parameters if needed
 *
 * @return void
 */
#define hcore_log_error(level, log, err, ...) \
    if ((log)->log_level >= level)            \
    hcore_log_error_core(level, log, err, __VA_ARGS__)

#ifdef _HCORE_DEBUG

/**
 * @brief record debug log
 *
 * @param log: log object
 * @param err: error code
 * @param ...: format string and parameters if needed
 *
 * @return void
 */
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
 * @brief  Convert log string to the corresponding integer value,
 * support: 'emerg', 'alert', 'crit', 'error', 'warn', 'notice', 'info',
 * 'debug';
 *
 * @note
 *
 * @param log_str: log string
 *
 * @retval * Upon successful completion, the function shall return the
 * corresponding integer value, such as macro "HCORE_LOG_ERROR", otherwise, the
 * function shall return HCORE_ERROR.
 */
hcore_int_t hcore_log_parse_level(const char *log_str);

/**
 * @brief  Output the text to STDERR
 *
 * @param text: text to output
 *
 * @retval void
 */
static inline void
hcore_write_stderr(char *text)
{
    (void)write(STDERR_FILENO, text, strlen(text));
}

/**
 * @brief  Output the text to STDOUT
 * 
 * @param  text: text to output
 * 
 * @retval None
 */
static inline void
hcore_write_stdout(char *text)
{
    (void)write(STDOUT_FILENO, text, strlen(text));
}

/**
 * @brief  Get the GMT time format string, such as: 2021/09/26 20:24:00 +0000 GMT
 * 
 * @note  The time is end with '\0'
 * 
 * @param  time[HCORE_LOG_TIME_LENGTH]: Used to store the time format string
 * 
 * @retval None
 */
void hcore_log_get_time(hcore_uchar_t time[HCORE_LOG_TIME_LENGTH]);

/**
 * @brief  Get the time format string of the current device, such as: 2021/09/26 20:24:00 +0800 CST 
 * 
 * @param  time[HCORE_LOG_TIME_LENGTH]: Used to store the time format string
 * 
 * @retval None
 */
void hcore_log_get_localtime(hcore_uchar_t time[HCORE_LOG_TIME_LENGTH]);

/**
 * @brief  create a log
 * 
 * @param  *pool: pool
 * @param  *log_file: log file path
 * @param  level: log level
 * 
 * @retval Upon successful return 'log', otherwise return 'NULL'
 */
hcore_log_t *hcore_create_log(hcore_pool_t *pool, char *log_file,
                              hcore_int_t level);

/**
 * @brief  destroy the log
 * 
 * @param  *log: log
 * 
 * @retval None
 */
void hcore_destroy_log(hcore_log_t *log);

/**
 * @brief  Open a log file
 * 
 * @note
 * @param  log : log object
 * @param  log_file: log file path
 * @param  level: log level
 *
 * @retval Upon successful return 'HCORE_OK', otherwise return 'HCORE_ERROR'
 */
hcore_int_t hcore_open_log(hcore_log_t *log, char *log_file, hcore_int_t level);

#endif // !_HCORE_LOG_H_INCLUDED_
