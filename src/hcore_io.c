/**
 * @file hcore_io.c
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

#include <hcore_io.h>

#include <errno.h>
#include <stdio.h>

FILE *
hcore_fopen(const hcore_uchar_t *path, const char *mode)
{
    return fopen((const char *)path, mode);
}

ssize_t
hcore_writev(int fd, const struct iovec *iov, int iovcnt)
{
    ssize_t n;

eintr:
    n = writev(fd, iov, iovcnt);

    if (n == -1 && errno == EAGAIN) goto eintr;

    return n;
}