/**
 * @file hcore_time.c
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

#include <hcore_time.h>

void
hcore_localtime(time_t s, hcore_tm_t *tm)
{
    localtime_r(&s, tm);

    tm->hcore_tm_mon++;
    tm->hcore_tm_year += 1900;
}

void
hcore_gmtime(time_t s, hcore_tm_t *tm)
{
    hcore_int_t  yday;
    hcore_uint_t sec, min, hour, mday, mon, year, wday, days, leap;

    /* the calculation is valid for positive time_t only */

    if (s < 0) { s = 0; }

    days = s / 86400;
    sec  = s % 86400;

    /*
     * no more than 4 year digits supported,
     * truncate to December 31, 9999, 23:59:59
     */

    if (days > 2932896)
    {
        days = 2932896;
        sec  = 86399;
    }

    /* January 1, 1970 was Thursday */

    wday = (4 + days) % 7;

    hour = sec / 3600;
    sec %= 3600;
    min = sec / 60;
    sec %= 60;

    /*
     * the algorithm based on Gauss' formula,
     * see src/core/ngx_parse_time.c
     */

    /* days since March 1, 1 BC */
    days = days - (31 + 28) + 719527;

    /*
     * The "days" should be adjusted to 1 only, however, some March 1st's go
     * to previous year, so we adjust them to 2.  This causes also shift of the
     * last February days to next year, but we catch the case when "yday"
     * becomes negative.
     */

    year = (days + 2) * 400 / (365 * 400 + 100 - 4 + 1);

    yday = days - (365 * year + year / 4 - year / 100 + year / 400);

    if (yday < 0)
    {
        leap = (year % 4 == 0) && (year % 100 || (year % 400 == 0));
        yday = 365 + leap + yday;
        year--;
    }

    /*
     * The empirical formula that maps "yday" to month.
     * There are at least 10 variants, some of them are:
     *     mon = (yday + 31) * 15 / 459
     *     mon = (yday + 31) * 17 / 520
     *     mon = (yday + 31) * 20 / 612
     */

    mon = (yday + 31) * 10 / 306;

    /* the Gauss' formula that evaluates days before the month */

    mday = yday - (367 * mon / 12 - 30) + 1;

    if (yday >= 306)
    {
        year++;
        mon -= 10;

        /*
         * there is no "yday" in Win32 SYSTEMTIME
         *
         * yday -= 306;
         */
    }
    else
    {
        mon += 2;

        /*
         * there is no "yday" in Win32 SYSTEMTIME
         *
         * yday += 31 + 28 + leap;
         */
    }

    tm->hcore_tm_sec  = (hcore_tm_sec_t)sec;
    tm->hcore_tm_min  = (hcore_tm_min_t)min;
    tm->hcore_tm_hour = (hcore_tm_hour_t)hour;
    tm->hcore_tm_mday = (hcore_tm_mday_t)mday;
    tm->hcore_tm_mon  = (hcore_tm_mon_t)mon;
    tm->hcore_tm_year = (hcore_tm_year_t)year;
    tm->hcore_tm_wday = (hcore_tm_wday_t)wday;
}

hcore_msec_t
hcore_monotonic_time()
{
    struct timespec ts;

#if defined(CLOCK_MONOTONIC_FAST)
    clock_gettime(CLOCK_MONOTONIC_FAST, &ts);

#elif defined(CLOCK_MONOTONIC_COARSE)
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);

#else
    clock_gettime(CLOCK_MONOTONIC, &ts);
#endif

    time_t     sec  = ts.tv_sec;
    hcore_uint_t msec = ts.tv_nsec / 1000000;

    return (hcore_msec_t)sec * 1000 + msec;
}

hcore_int_t
hcore_parse_time(hcore_str_t *line, hcore_uint_t is_sec)
{
    hcore_uchar_t *p, *last;
    hcore_int_t    value, total, scale;
    hcore_int_t    max, cutoff, cutlim;
    hcore_uint_t   valid;
    enum
    {
        st_start = 0,
        st_year,
        st_month,
        st_week,
        st_day,
        st_hour,
        st_min,
        st_sec,
        st_msec,
        st_last
    } step;

    valid  = 0;
    value  = 0;
    total  = 0;
    cutoff = HCORE_MAX_INT_T_VALUE / 10;
    cutlim = HCORE_MAX_INT_T_VALUE % 10;
    step   = is_sec ? st_start : st_month;

    p    = line->data;
    last = p + line->len;

    while (p < last)
    {
        if (*p >= '0' && *p <= '9')
        {
            if (value >= cutoff && (value > cutoff || *p - '0' > cutlim))
            {
                return HCORE_ERROR;
            }

            value = value * 10 + (*p++ - '0');
            valid = 1;
            continue;
        }

        switch (*p++)
        {
        case 'y':
            if (step > st_start) { return HCORE_ERROR; }
            step  = st_year;
            max   = HCORE_MAX_INT_T_VALUE / (60 * 60 * 24 * 365);
            scale = 60 * 60 * 24 * 365;
            break;

        case 'M':
            if (step >= st_month) { return HCORE_ERROR; }
            step  = st_month;
            max   = HCORE_MAX_INT_T_VALUE / (60 * 60 * 24 * 30);
            scale = 60 * 60 * 24 * 30;
            break;

        case 'w':
            if (step >= st_week) { return HCORE_ERROR; }
            step  = st_week;
            max   = HCORE_MAX_INT_T_VALUE / (60 * 60 * 24 * 7);
            scale = 60 * 60 * 24 * 7;
            break;

        case 'd':
            if (step >= st_day) { return HCORE_ERROR; }
            step  = st_day;
            max   = HCORE_MAX_INT_T_VALUE / (60 * 60 * 24);
            scale = 60 * 60 * 24;
            break;

        case 'h':
            if (step >= st_hour) { return HCORE_ERROR; }
            step  = st_hour;
            max   = HCORE_MAX_INT_T_VALUE / (60 * 60);
            scale = 60 * 60;
            break;

        case 'm':
            if (p < last && *p == 's')
            {
                if (is_sec || step >= st_msec) { return HCORE_ERROR; }
                p++;
                step  = st_msec;
                max   = HCORE_MAX_INT_T_VALUE;
                scale = 1;
                break;
            }

            if (step >= st_min) { return HCORE_ERROR; }
            step  = st_min;
            max   = HCORE_MAX_INT_T_VALUE / 60;
            scale = 60;
            break;

        case 's':
            if (step >= st_sec) { return HCORE_ERROR; }
            step  = st_sec;
            max   = HCORE_MAX_INT_T_VALUE;
            scale = 1;
            break;

        case ' ':
            if (step >= st_sec) { return HCORE_ERROR; }
            step  = st_last;
            max   = HCORE_MAX_INT_T_VALUE;
            scale = 1;
            break;

        default: return HCORE_ERROR;
        }

        if (step != st_msec && !is_sec)
        {
            scale *= 1000;
            max /= 1000;
        }

        if (value > max) { return HCORE_ERROR; }

        value *= scale;

        if (total > HCORE_MAX_INT_T_VALUE - value) { return HCORE_ERROR; }

        total += value;

        value = 0;

        while (p < last && *p == ' ') { p++; }
    }

    if (!valid) { return HCORE_ERROR; }

    if (!is_sec)
    {
        if (value > HCORE_MAX_INT_T_VALUE / 1000) { return HCORE_ERROR; }

        value *= 1000;
    }

    if (total > HCORE_MAX_INT_T_VALUE - value) { return HCORE_ERROR; }

    return total + value;
}
