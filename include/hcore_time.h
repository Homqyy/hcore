/**
 * @file hcore_time.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief 提供时间接口
 * @version 0.1
 * @date 2021-09-26
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#ifndef _HCORE_TIME_H_INCLUDED_
#define _HCORE_TIME_H_INCLUDED_

#include <hcore_constant.h>
#include <hcore_string.h>

#include <time.h>

typedef struct tm hcore_tm_t;

#define hcore_tm_sec    tm_sec
#define hcore_tm_min    tm_min
#define hcore_tm_hour   tm_hour
#define hcore_tm_mday   tm_mday
#define hcore_tm_mon    tm_mon
#define hcore_tm_year   tm_year
#define hcore_tm_wday   tm_wday
#define hcore_tm_isdst  tm_isdst
#define hcore_tm_gmtoff tm_gmtoff

#define hcore_tm_sec_t  int
#define hcore_tm_min_t  int
#define hcore_tm_hour_t int
#define hcore_tm_mday_t int
#define hcore_tm_mon_t  int
#define hcore_tm_year_t int
#define hcore_tm_wday_t int

extern long  timezone;
extern char *tzname[2];

#define hcore_gettimeofday(tv) gettimeofday(tv, NULL)
#define hcore_tz_offset        timezone
#define hcore_tzname           tzname[0]

void       hcore_gmtime(time_t s, hcore_tm_t *tm);
void       hcore_localtime(time_t s, hcore_tm_t *tm);
hcore_msec_t hcore_monotonic_time();
hcore_int_t  hcore_parse_time(hcore_str_t *line, hcore_uint_t is_sec);

#endif // !_HCORE_TIME_H_INCLUDED_
