/**
 * @file hcore_io.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief sdtio.h 的包裹函数
 * @version 0.1
 * @date 2021-09-26
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#ifndef _HCORE_IO_H_INCLUDED_
#define _HCORE_IO_H_INCLUDED_

#include <hcore_types.h>

#include <stdio.h>
#include <sys/uio.h>

FILE *  hcore_fopen(const hcore_uchar_t *path, const char *mode);
ssize_t hcore_writev(int fd, const struct iovec *iov, int iovcnt);

#endif // !_HCORE_IO_H_INCLUDED_
