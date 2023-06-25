/**
 * @file hcore_count.c
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

#include <hcore_count.h>

hcore_int_t
hcore_multip_is_overflow(int a, int b)
{
    int y = a * b;

    return !(b == 0 || y / b == a);
}

hcore_int_t
hcore_multip_is_overflow_long(long a, long b)
{
    long y = a * b;

    return !(b == 0 || y / b == a);
}
