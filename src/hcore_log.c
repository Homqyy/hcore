/**
 * @file hcore_log.c
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief
 * @version 0.1
 * @date 2021-10-18
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#include <hcore_base.h>
#include <hcore_debug.h>
#include <hcore_log.h>
#include <hcore_pool.h>
#include <hcore_string.h>
#include <hcore_time.h>

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

struct
{
    int         level;
    hcore_str_t name;
    hcore_str_t padding;
} static g_hcore_log_level[] = {
    {HCORE_LOG_STDERR, hcore_string("stderr"), hcore_string("")},
    {HCORE_LOG_EMERG, hcore_string("emerg"), hcore_string(" ")},
    {HCORE_LOG_ALERT, hcore_string("alert"), hcore_string(" ")},
    {HCORE_LOG_CRIT, hcore_string("crit"), hcore_string("  ")},
    {HCORE_LOG_ERR, hcore_string("error"), hcore_string(" ")},
    {HCORE_LOG_WARN, hcore_string("warn"), hcore_string("  ")},
    {HCORE_LOG_NOTICE, hcore_string("notice"), hcore_string("")},
    {HCORE_LOG_INFO, hcore_string("info"), hcore_string("  ")},
    {HCORE_LOG_DEBUG, hcore_string("debug"), hcore_string(" ")},
};


hcore_int_t
hcore_log_parse_level(const char *log_str)
{
    if (log_str == NULL)
    {
        return HCORE_ERROR;
    }

    int i;
    int len = strlen(log_str);

    if (len == 0)
    {
        return HCORE_ERROR;
    }

    for (i = 0; i < HCORE_ARRAY_NUM(g_hcore_log_level); i++)
    {
        if (g_hcore_log_level[i].name.len == len
            && hcore_memcmp(g_hcore_log_level[i].name.data, log_str, len) == 0)
        {
            return g_hcore_log_level[i].level;
        }
    }

    return HCORE_ERROR;
}

/* format:
 *     time [level] -pid- <object>: (errno) message
 */
void
hcore_log_error_core(int level, hcore_log_t *log, hcore_err_t err,
                     const char *fmt, ...)
{
    hcore_uchar_t *p, *last;
    hcore_uchar_t  errstr[HCORE_LOG_ERRSTR_LENGTH_MAX] = {0};
    hcore_uchar_t  time[HCORE_LOG_TIME_LENGTH];
    char           errno_buf[64];
    va_list        args;

    hcore_assert(HCORE_LOG_STDERR <= level && level <= HCORE_LOG_DEBUG);

    p    = errstr;
    last = errstr + HCORE_LOG_ERRSTR_LENGTH_MAX;

    if (log->get_time)
    {
        log->get_time(time);
    }
    else
    {
        hcore_log_get_localtime(time);
    }

    p = hcore_slprintf(p, last, "%s%Z", time);

    p = hcore_slprintf(p, last, " [%V%V]", &g_hcore_log_level[level].padding,
                       &g_hcore_log_level[level].name);

    p = hcore_slprintf(p, last, " %5P", getpid());

    p = hcore_slprintf(p, last, " %s:", log->object ? log->object : "unknown");

    if (err)
    {
#ifdef _GNU_SOURCE
        char *msg = strerror_r(err, errno_buf, sizeof(errno_buf));

        if (msg == NULL)
        {
            if (errno == EINVAL)
            {
                hcore_memcpy(errno_buf, "parse error: INVAL",
                             sizeof("parse error: INVAL"));
            }
            else if (errno == ERANGE)
            {
                hcore_memcpy(errno_buf, "parse error: RANGE",
                             sizeof("parse error: RANGE"));
            }
            else
            {
                hcore_memcpy(errno_buf, "no parsed", sizeof("no parsed"));
            }

            msg = errno_buf;
        }

        p = hcore_slprintf(p, last, " (%d: %s) ", err, msg);
#else
        if (strerror_r(err, errno_buf, sizeof(errno_buf)) != 0)
        {
            if (errno == EINVAL)
            {
                hcore_memcpy(errno_buf, "parse error: INVAL",
                             sizeof("parse error: INVAL"));
            }
            else if (errno == ERANGE)
            {
                hcore_memcpy(errno_buf, "parse error: RANGE",
                             sizeof("parse error: RANGE"));
            }
            else
            {
                hcore_memcpy(errno_buf, "no parsed", sizeof("no parsed"));
            }
        }

        p = hcore_slprintf(p, last, " (%d: %s) ", err, errno_buf);
#endif
    }
    else
    {
        p = hcore_slprintf(p, last, " (%d) ", err);
    }

    va_start(args, fmt);
    p = hcore_vslprintf(p, last, fmt, args);
    va_end(args);

    if (log->handler) p = log->handler(log, p, last);

    if (p > last - HCORE_LINEFEED_SIZE)
    {
        p = last - HCORE_LINEFEED_SIZE;
    }

    *p++ = HCORE_LF;

    write(log->fd, errstr, p - errstr);
}

void
hcore_log_get_time(hcore_uchar_t time[HCORE_LOG_TIME_LENGTH])
{
    struct timeval tv;
    hcore_tm_t     gmt;

    (void)hcore_gettimeofday(&tv);

    hcore_gmtime(tv.tv_sec, &gmt);

    (void)hcore_snprintf(time, HCORE_LOG_TIME_LENGTH,
                         "%4d/%02d/%02d %02d:%02d:%02d +0000 GMT%Z",
                         gmt.hcore_tm_year, gmt.hcore_tm_mon, gmt.hcore_tm_mday,
                         gmt.hcore_tm_hour, gmt.hcore_tm_min, gmt.hcore_tm_sec);
}

void
hcore_log_get_localtime(hcore_uchar_t time[HCORE_LOG_TIME_LENGTH])
{
    struct timeval tv;
    hcore_tm_t     tm;
    hcore_int_t    gmtoff_m;

    (void)hcore_gettimeofday(&tv);

    hcore_localtime(tv.tv_sec, &tm);

    gmtoff_m = tm.hcore_tm_gmtoff / 60; // minute

    (void)hcore_snprintf(
        time, HCORE_LOG_TIME_LENGTH,
        "%4d/%02d/%02d %02d:%02d:%02d %c%02d%02d %s%Z", tm.hcore_tm_year,
        tm.hcore_tm_mon, tm.hcore_tm_mday, tm.hcore_tm_hour, tm.hcore_tm_min,
        tm.hcore_tm_sec, tm.hcore_tm_gmtoff > 0 ? '+' : '-',
        hcore_abs(gmtoff_m / 60), hcore_abs(gmtoff_m % 60), hcore_tzname);
}

hcore_int_t
hcore_open_log(hcore_log_t *log, char *log_file, hcore_int_t level)
{
    hcore_assert(log && log_file);

    int          fd       = -1;
    hcore_uint_t internal = 0;

    if (level < HCORE_LOG_STDERR || level > HCORE_LOG_DEBUG) return HCORE_ERROR;

    if (strncmp(log_file, HCORE_LOG_FILE_STDOUT, sizeof(HCORE_LOG_FILE_STDOUT))
        == 0)
    {
        fd       = dup(STDOUT_FILENO);
        internal = 1;
    }
    else if (strncmp(log_file, HCORE_LOG_FILE_STDERR,
                     sizeof(HCORE_LOG_FILE_STDERR))
             == 0)
    {
        fd       = dup(STDERR_FILENO);
        internal = 1;
    }
    else
    {
        fd = open(log_file, O_CREAT | O_APPEND | O_RDWR, S_IWUSR | S_IRUSR);
        if (fd == -1)
        {
            fprintf(stderr, "open %s error: %s\r\n", log_file, strerror(errno));
            return HCORE_ERROR;
        }
    }

    hcore_memzero(log, sizeof(hcore_log_t));

    log->fd        = fd;
    log->log_level = level;
    log->filename  = log_file;
    log->internal  = internal;

    return HCORE_OK;
}

hcore_log_t *
hcore_create_log(hcore_pool_t *pool, char *log_file, hcore_int_t level)
{
    hcore_log_t *log;

    hcore_assert(pool && log_file);

    log = hcore_pcalloc(pool, sizeof(hcore_log_t));
    if (log == NULL) return NULL;

    if (hcore_open_log(log, log_file, level) != HCORE_OK) return NULL;

    hcore_pool_cleanup_t *cln = hcore_pool_cleanup_add(pool, 0);
    if (cln == NULL) return NULL;

    cln->data    = log;
    cln->handler = (hcore_pool_clean_handler_pt)hcore_destroy_log;

    return log;
}

void
hcore_destroy_log(hcore_log_t *log)
{
    if (close(log->fd) == -1)
    {
        fprintf(stderr, "close fd(#%d) of '%s' failed: %s\r\n", log->fd,
                log->filename, strerror(errno));
    }
}