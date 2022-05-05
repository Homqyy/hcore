/**
 * @file hcore_log.c
 * @author homqyy (yilupiaoxuewhq@163.com)
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
#include <hcore_log.h>
#include <hcore_string.h>
#include <hcore_time.h>

#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>

static hcore_str_t g_hcore_err_levels[] = {
    hcore_null_string,      hcore_string("emerg"), hcore_string("alert"),
    hcore_string("crit"),   hcore_string("error"), hcore_string("warn"),
    hcore_string("notice"), hcore_string("info"),  hcore_string("debug")};

static hcore_str_t g_hcore_err_padding[] = {
    hcore_string(""),   hcore_string(" "),  hcore_string(" "),
    hcore_string("  "), hcore_string(" "),  hcore_string("  "),
    hcore_string(""),   hcore_string("  "), hcore_string(" ")};

hcore_int_t
hcore_log_parse_level(const char *log_str)
{
    if (log_str == NULL) { return HCORE_FAILED; }

    int i;
    int len = strlen(log_str);

    for (i = 1; i <= HCORE_LOG_DEBUG; i++)
    {
        if (g_hcore_err_levels[i].len == len
            && hcore_memcmp(g_hcore_err_levels[i].data, log_str, len) == 0)
        {
            return i;
        }
    }

    return HCORE_FAILED;
}

/* format:
 *     time [level] -pid- <object>: (errno) message
 */
void
hcore_log_error_core(int level, hcore_log_t *log, hcore_err_t err, const char *fmt,
                   ...)
{
    hcore_uchar_t *p, *last;
    hcore_uchar_t  errstr[HCORE_LOG_ERRSTR_LENGTH_MAX];
    hcore_uchar_t  time[HCORE_LOG_TIME_LENGTH];
    va_list      args;

    p    = errstr;
    last = errstr + HCORE_LOG_ERRSTR_LENGTH_MAX;

    if (log->get_time) { log->get_time(time); }
    else
    {
        hcore_log_get_localtime(time);
    }

    p = hcore_slprintf(p, last, "%s%Z", time);

    p = hcore_slprintf(p, last, " [%V%V]", &g_hcore_err_padding[level],
                     &g_hcore_err_levels[level]);

    p = hcore_slprintf(p, last, " %5P", getpid());

    p = hcore_slprintf(p, last, " %s:", log->object ? log->object : "unknown");

    p = hcore_slprintf(p, last, " (%d) ", err);

    va_start(args, fmt);
    p = hcore_vslprintf(p, last, fmt, args);
    va_end(args);

    if (log->handler) p = log->handler(log, p, last);

    if (p > last - HCORE_LINEFEED_SIZE) { p = last - HCORE_LINEFEED_SIZE; }

    *p++ = HCORE_LF;

    write(log->fd, errstr, p - errstr);
}

void
hcore_log_get_time(hcore_uchar_t time[HCORE_LOG_TIME_LENGTH])
{
    struct timeval tv;
    hcore_tm_t       gmt;

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
    hcore_tm_t       tm;
    hcore_int_t      gmtoff_m;

    (void)hcore_gettimeofday(&tv);

    hcore_localtime(tv.tv_sec, &tm);

    gmtoff_m = tm.hcore_tm_gmtoff / 60; // minute

    (void)hcore_snprintf(time, HCORE_LOG_TIME_LENGTH,
                       "%4d/%02d/%02d %02d:%02d:%02d %c%02d%02d %s%Z",
                       tm.hcore_tm_year, tm.hcore_tm_mon, tm.hcore_tm_mday,
                       tm.hcore_tm_hour, tm.hcore_tm_min, tm.hcore_tm_sec,
                       tm.hcore_tm_gmtoff > 0 ? '+' : '-', hcore_abs(gmtoff_m / 60),
                       hcore_abs(gmtoff_m % 60), hcore_tzname);
}
