/**
 * @file hcore_string.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief
 * @version 0.1
 * @date 2021-09-22
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#ifndef _HCORE_STRING_H_INCLUDED_
#define _HCORE_STRING_H_INCLUDED_

#include <hcore_array.h>
#include <hcore_types.h>

#include <stdarg.h>
#include <string.h>

typedef struct
{
    size_t         len;
    hcore_uchar_t *data;
} hcore_str_t;

#define hcore_string(string)                        \
    {                                               \
        sizeof(string) - 1, (hcore_uchar_t *)string \
    }

#define hcore_null_string \
    {                     \
        0, NULL           \
    }

#define hcore_str_set(str, text)    \
    (str)->len  = sizeof(text) - 1; \
    (str)->data = (hcore_uchar_t *)text
#define hcore_str_null(str) \
    (str)->len  = 0;        \
    (str)->data = NULL

#define hcore_memzero(buf, n)      (void)memset(buf, 0, n)
#define hcore_memcpy(dst, src, n)  (void)memcpy(dst, src, n)
#define hcore_cpymem(dst, src, n)  (((hcore_uchar_t *)memcpy(dst, src, n)) + (n))
#define hcore_memmove(dst, src, n) (void)memmove(dst, src, n)
#define hcore_movemem(dst, src, n) \
    (((hcore_uchar_t *)memmove(dst, src, n)) + (n))
#define hcore_memcmp(s1, s2, n) memcmp((const char *)s1, (const char *)s2, n)
#define hcore_strlen(s)         strlen((const char *)s)
#define hcore_strnlen(s, n)     strnlen((const char *)s, n);

static inline hcore_uchar_t *
hcore_strlchr(hcore_uchar_t *p, hcore_uchar_t *last, char c)
{
    while (p < last)
    {
        if (*p == c)
        {
            return p;
        }

        p++;
    }

    return NULL;
}

ssize_t      hcore_atosz(hcore_uchar_t *line, size_t n);
hcore_int_t  hcore_atoi(hcore_uchar_t *line, size_t n);
hcore_int_t  hcore_hextoi(hcore_uchar_t *line, size_t n);
/**
 * @brief parse string of size to integer, such as "1024", "5K", "10M", "2G".
 *
 * @param line string of size
 * @return ssize_t : Upon successful is return integer expression of string
 * 'line', otherwise return HCORE_ERROR to meant error occur.
 */
ssize_t      hcore_parse_size(hcore_str_t *line);
/**
 * @brief parse string of version to integer, such as "v1.0.0", "1.0.0".
 * @note string format is "[v|V]<master>.<minor>.<patch>". valut of the
 * <master>, <minor> and <patch> must in [1, 255]. prefix 'v' or 'V' is option.
 *
 * @param line string of version
 * @return hcore_uint_t Upon successful is return integer expression of version
 * string, otherwise return 0.
 */
hcore_uint_t hcore_parse_version(hcore_str_t *line);

/**
 * @brief format 'size' as string in ‘buff'(buffer)
 * @note
 * @param  size: size that need be format as string, and unit is 'byte'
 * @param  *buff: buffer
 * @param  *last: last of buffer
 * @retval
 * * last of string
 */
hcore_uchar_t *hcore_strlfmt_size(ssize_t size, hcore_uchar_t *buff,
                                  hcore_uchar_t *last);

#define hcore_strfmt_size(size, buff, buff_len) \
    hcore_strlfmt_size(size, buff, buff + buff_len)

/**
 * @brief
 * @note
 * @param  *buf:
 * @param  *last:
 * @param  *fmt:
 * * %L => hcore_int64_t
 * * %z => ssize_t
 * * %f => double
 * * %V => hcore_str_t *
 * * %d => int
 * * %x => hex
 * * %x => capitalization hex
 * * %z => ssize_t
 * * %s => char *
 * * %P => hcore_pid_t
 * * %i => hcore_int_t
 * * %O => off_t
 * * %T => time_t
 * * %M => hcore_msec_t
 * * %l => long
 * * %D => hcore_int32_t
 * * %p => void * for pointer address
 * * %c => single char
 * * %N => '\n'
 * * %% => '%'
 * *
 * * %* => string length
 * * %u => unsigned
 * * %Z => addon '\0'
 * @param  args:
 * @retval
 */
hcore_uchar_t *hcore_vslprintf(hcore_uchar_t *buf, hcore_uchar_t *last,
                               const char *fmt, va_list args);
hcore_uchar_t *hcore_slprintf(hcore_uchar_t *buf, hcore_uchar_t *last,
                              const char *fmt, ...);
hcore_uchar_t *hcore_hex_dump(hcore_uchar_t *dst, hcore_uchar_t *src,
                              size_t len);
hcore_uchar_t *hcore_snprintf(u_char *buf, size_t max, const char *fmt, ...);
hcore_array_t *hcore_strtok(hcore_pool_t *pool, hcore_str_t *src, char *delim);
/**
 * @brief
 * @note
 * @param  args:
 * @retval
 */
hcore_array_t *hcore_strtokz(hcore_pool_t *pool, hcore_str_t *src, char *delim);

/**
 * @brief deeply (allocate memory by pool) copy string
 * @note
 * @param  args:
 * @retval
 */
char *hcore_strcpyd(hcore_pool_t *pool, const char *string);


#endif // !_HCORE_STRING_H_INCLUDED_
