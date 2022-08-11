/**
 * @file hcore_string.c
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
#include <hcore_constant.h>
#include <hcore_debug.h>
#include <hcore_string.h>

#include <stdarg.h>

static hcore_uchar_t *hcore_sprintf_num(hcore_uchar_t *buf, hcore_uchar_t *last,
                                        hcore_uint64_t ui64, hcore_uchar_t zero,
                                        hcore_uint_t hexadecimal,
                                        hcore_uint_t width);

hcore_uchar_t *
hcore_strlfmt_size(ssize_t size, hcore_uchar_t *buff, hcore_uchar_t *last)
{
    const char  *units[] = {"B", "KB", "MB", "GB", "TB", NULL};
    const char **pu;
    ssize_t      scale, v;
    double       decimal;
    double       r; // remainder

    pu    = units;
    v     = size;
    scale = 1;

    while (v)
    {
        if (!(v >> 10)) break;

        pu++;
        scale <<= 10;
        v >>= 10;
    }

    r = size & (scale - 1);

    decimal = r / scale;
    decimal += v;

    return hcore_slprintf(buff, last, "%.02f %s", decimal, *pu);
}

ssize_t
hcore_parse_size(hcore_str_t *line)
{
    u_char  unit;
    size_t  len;
    ssize_t size, scale, max;

    len = line->len;

    if (len == 0)
    {
        return HCORE_ERROR;
    }

    unit = line->data[len - 1];

    switch (unit)
    {
    case 'K':
    case 'k':
        len--;
        max   = HCORE_MAX_SSIZE_T_VALUE / 1024;
        scale = 1024;
        break;

    case 'M':
    case 'm':
        len--;
        max   = HCORE_MAX_SSIZE_T_VALUE / (1024 * 1024);
        scale = 1024 * 1024;
        break;

    case 'G':
    case 'g':
        len--;
        max   = HCORE_MAX_SSIZE_T_VALUE / (1024 * 1024 * 1024);
        scale = 1024 * 1024 * 1024;
        break;

    default: max = HCORE_MAX_SSIZE_T_VALUE; scale = 1;
    }

    size = hcore_atosz(line->data, len);
    if (size == HCORE_ERROR || size > max)
    {
        return HCORE_ERROR;
    }

    size *= scale;

    return size;
}

hcore_uint_t
hcore_parse_version(hcore_str_t *line)
{
    // v1.0.0

    enum
    {
        prefix_phase,
        separator_phase,
        version_phase,
    };

    hcore_uchar_t *p     = line->data;
    hcore_uchar_t *last  = line->data + line->len;
    hcore_uint_t   phase = prefix_phase;

    hcore_uint_t master = 0;
    hcore_uint_t minor  = 0;
    hcore_uint_t patch  = 0;

    hcore_uint_t *vers[4] = {&master, &minor, &patch, NULL};
    hcore_uint_t  ver_pos = 0; // version position

    while (p < last)
    {
        char c = *p;
        switch (phase)
        {
        case prefix_phase:
            if (c == 'v' || c == 'V')
            {
                p++;
            }

            phase = version_phase;
            break;

        case version_phase:
            if (vers[ver_pos] == NULL) break; // complete version

            while (1)
            {
                if ('0' <= c && c <= '9')
                {
                    *vers[ver_pos] = *vers[ver_pos] * 10 + (c - '0');

                    if (0xff < *vers[ver_pos])
                    {
                        goto error;
                    }

                    p++;
                    c = *p;
                }
                else
                {
                    ver_pos++;
                    phase = separator_phase;
                    break;
                }
            }
            break;

        case separator_phase:
            if (c != '.')
            {
                goto error;
            }

            phase = version_phase;
            break;
        }
    }

    if (master == 0) goto error;

    return master << 16 | minor << 8 | patch;

error:
    return 0; // '0' express error
}

ssize_t
hcore_atosz(hcore_uchar_t *line, size_t n)
{
    ssize_t value, cutoff, cutlim;

    if (n == 0)
    {
        return HCORE_ERROR;
    }

    cutoff = HCORE_MAX_SSIZE_T_VALUE / 10;
    cutlim = HCORE_MAX_SSIZE_T_VALUE % 10;

    for (value = 0; n--; line++)
    {
        if (*line < '0' || *line > '9')
        {
            return HCORE_ERROR;
        }

        if (value >= cutoff && (value > cutoff || *line - '0' > cutlim))
        {
            return HCORE_ERROR;
        }

        value = value * 10 + (*line - '0');
    }

    return value;
}

hcore_int_t
hcore_atoi(hcore_uchar_t *line, size_t n)
{
    hcore_int_t value, cutoff, cutlim;

    if (n == 0)
    {
        return HCORE_ERROR;
    }

    cutoff = HCORE_MAX_INT_T_VALUE / 10;
    cutlim = HCORE_MAX_INT_T_VALUE % 10;

    for (value = 0; n--; line++)
    {
        if (*line < '0' || *line > '9')
        {
            return HCORE_ERROR;
        }

        if (value >= cutoff && (value > cutoff || *line - '0' > cutlim))
        {
            return HCORE_ERROR;
        }

        value = value * 10 + (*line - '0');
    }

    return value;
}

hcore_int_t
hcore_hextoi(hcore_uchar_t *line, size_t n)
{
    hcore_uchar_t c, ch;
    hcore_int_t   value, cutoff;

    if (n == 0)
    {
        return HCORE_ERROR;
    }

    cutoff = HCORE_MAX_INT_T_VALUE / 16;

    for (value = 0; n--; line++)
    {
        if (value > cutoff)
        {
            return HCORE_ERROR;
        }

        ch = *line;

        if (ch >= '0' && ch <= '9')
        {
            value = value * 16 + (ch - '0');
            continue;
        }

        c = (hcore_uchar_t)(ch | 0x20);

        if (c >= 'a' && c <= 'f')
        {
            value = value * 16 + (c - 'a' + 10);
            continue;
        }

        return HCORE_ERROR;
    }

    return value;
}

hcore_uchar_t *
hcore_hex_dump(hcore_uchar_t *dst, hcore_uchar_t *src, size_t len)
{
    static hcore_uchar_t hex[] = "0123456789abcdef";

    while (len--)
    {
        *dst++ = hex[*src >> 4];
        *dst++ = hex[*src++ & 0xf];
    }

    return dst;
}

hcore_uchar_t *
hcore_vslprintf(hcore_uchar_t *buf, hcore_uchar_t *last, const char *fmt,
                va_list args)
{
    hcore_uchar_t *p, zero;
    int            d;
    double         f;
    size_t         len, slen;
    hcore_int64_t  i64;
    hcore_uint64_t ui64, frac;
    hcore_uint_t   width, sign, hex, max_width, frac_width, scale, n;
    hcore_str_t   *v;
    hcore_msec_t   ms;

    while (*fmt && buf < last)
    {
        /*
         * "buf < last" means that we could copy at least one character:
         * the plain character, "%%", "%c", and minus without the checking
         */

        if (*fmt == '%')
        {
            i64  = 0;
            ui64 = 0;

            zero       = (hcore_uchar_t)((*++fmt == '0') ? '0' : ' ');
            width      = 0;
            sign       = 1;
            hex        = 0;
            max_width  = 0;
            frac_width = 0;
            slen       = (size_t)-1;

            while (*fmt >= '0' && *fmt <= '9')
            {
                width = width * 10 + (*fmt++ - '0');
            }


            for (;;)
            {
                switch (*fmt)
                {
                case 'u':
                    sign = 0;
                    fmt++;
                    continue;

                case 'm':
                    max_width = 1;
                    fmt++;
                    continue;

                case 'X':
                    hex  = 2;
                    sign = 0;
                    fmt++;
                    continue;

                case 'x':
                    hex  = 1;
                    sign = 0;
                    fmt++;
                    continue;

                case '.':
                    fmt++;

                    while (*fmt >= '0' && *fmt <= '9')
                    {
                        frac_width = frac_width * 10 + (*fmt++ - '0');
                    }

                    break;

                case '*':
                    slen = va_arg(args, size_t);
                    fmt++;
                    continue;

                default: break;
                }

                break;
            }


            switch (*fmt)
            {
            case 'V':
                v = va_arg(args, hcore_str_t *);

                len = hcore_min(((size_t)(last - buf)), v->len);
                buf = hcore_cpymem(buf, v->data, len);
                fmt++;

                continue;

            case 's':
                p = va_arg(args, hcore_uchar_t *);

                if (slen == (size_t)-1)
                {
                    while (*p && buf < last)
                    {
                        *buf++ = *p++;
                    }
                }
                else
                {
                    len = hcore_min(((size_t)(last - buf)), slen);
                    buf = hcore_cpymem(buf, p, len);
                }

                fmt++;

                continue;

            case 'O':
                i64  = (hcore_int64_t)va_arg(args, off_t);
                sign = 1;
                break;

            case 'P':
                i64  = (int64_t)va_arg(args, hcore_pid_t);
                sign = 1;
                break;

            case 'T':
                i64  = (hcore_int64_t)va_arg(args, time_t);
                sign = 1;
                break;

            case 'M':
                ms = (hcore_msec_t)va_arg(args, hcore_msec_t);
                if ((hcore_msec_int_t)ms == -1)
                {
                    sign = 1;
                    i64  = -1;
                }
                else
                {
                    sign = 0;
                    ui64 = (hcore_uint64_t)ms;
                }
                break;

            case 'z':
                if (sign)
                {
                    i64 = (hcore_int64_t)va_arg(args, ssize_t);
                }
                else
                {
                    ui64 = (hcore_uint64_t)va_arg(args, size_t);
                }
                break;

            case 'i':
                if (sign)
                {
                    i64 = (hcore_int64_t)va_arg(args, hcore_int_t);
                }
                else
                {
                    ui64 = (hcore_uint64_t)va_arg(args, hcore_uint_t);
                }

                if (max_width)
                {
                    width = HCORE_INT_T_LEN;
                }

                break;

            case 'd':
                if (sign)
                {
                    i64 = (hcore_int64_t)va_arg(args, int);
                }
                else
                {
                    ui64 = (hcore_uint64_t)va_arg(args, u_int);
                }
                break;

            case 'l':
                if (sign)
                {
                    i64 = (hcore_int64_t)va_arg(args, long);
                }
                else
                {
                    ui64 = (hcore_uint64_t)va_arg(args, u_long);
                }
                break;

            case 'D':
                if (sign)
                {
                    i64 = (hcore_int64_t)va_arg(args, hcore_int32_t);
                }
                else
                {
                    ui64 = (hcore_uint64_t)va_arg(args, hcore_uint32_t);
                }
                break;

            case 'L':
                if (sign)
                {
                    i64 = va_arg(args, hcore_int64_t);
                }
                else
                {
                    ui64 = va_arg(args, hcore_uint64_t);
                }
                break;

            case 'f':
                f = va_arg(args, double);

                if (f < 0)
                {
                    *buf++ = '-';
                    f      = -f;
                }

                ui64 = (hcore_int64_t)f;
                frac = 0;

                if (frac_width)
                {
                    scale = 1;
                    for (n = frac_width; n; n--)
                    {
                        scale *= 10;
                    }

                    frac = (hcore_uint64_t)((f - (double)ui64) * scale + 0.5);

                    if (frac == scale)
                    {
                        ui64++;
                        frac = 0;
                    }
                }

                buf = hcore_sprintf_num(buf, last, ui64, zero, 0, width);

                if (frac_width)
                {
                    if (buf < last)
                    {
                        *buf++ = '.';
                    }

                    buf =
                        hcore_sprintf_num(buf, last, frac, '0', 0, frac_width);
                }

                fmt++;

                continue;

            case 'p':
                ui64  = (uintptr_t)va_arg(args, void *);
                hex   = 2;
                sign  = 0;
                zero  = '0';
                width = 2 * sizeof(void *);
                break;

            case 'c':
                d      = va_arg(args, int);
                *buf++ = (hcore_uchar_t)(d & 0xff);
                fmt++;

                continue;

            case 'Z':
                *buf++ = '\0';
                fmt++;

                continue;

            case 'N':
                *buf++ = HCORE_LF;
                fmt++;

                continue;

            case '%':
                *buf++ = '%';
                fmt++;

                continue;

            default: *buf++ = *fmt++; continue;
            }

            if (sign)
            {
                if (i64 < 0)
                {
                    *buf++ = '-';
                    ui64   = (hcore_uint64_t)-i64;
                }
                else
                {
                    ui64 = (hcore_uint64_t)i64;
                }
            }

            buf = hcore_sprintf_num(buf, last, ui64, zero, hex, width);

            fmt++;
        }
        else
        {
            *buf++ = *fmt++;
        }
    }

    return buf;
}

static hcore_uchar_t *
hcore_sprintf_num(hcore_uchar_t *buf, hcore_uchar_t *last, hcore_uint64_t ui64,
                  hcore_uchar_t zero, hcore_uint_t hexadecimal,
                  hcore_uint_t width)
{
    hcore_uchar_t       *p, temp[HCORE_INT64_LEN + 1];
    /*
     * we need temp[NGX_INT64_LEN] only,
     * but icc issues the warning
     */
    size_t               len;
    hcore_uint32_t       ui32;
    static hcore_uchar_t hex[] = "0123456789abcdef";
    static hcore_uchar_t HEX[] = "0123456789ABCDEF";

    p = temp + HCORE_INT64_LEN;

    if (hexadecimal == 0)
    {
        if (ui64 <= (uint64_t)HCORE_MAX_UINT32_VALUE)
        {
            /*
             * To divide 64-bit numbers and to find remainders
             * on the x86 platform gcc and icc call the libc functions
             * [u]divdi3() and [u]moddi3(), they call another function
             * in its turn.  On FreeBSD it is the qdivrem() function,
             * its source code is about 170 lines of the code.
             * The glibc counterpart is about 150 lines of the code.
             *
             * For 32-bit numbers and some divisors gcc and icc use
             * a inlined multiplication and shifts.  For example,
             * unsigned "i32 / 10" is compiled to
             *
             *     (i32 * 0xCCCCCCCD) >> 35
             */

            ui32 = (hcore_uint32_t)ui64;

            do
            {
                *--p = (hcore_uchar_t)(ui32 % 10 + '0');
            } while (ui32 /= 10);
        }
        else
        {
            do
            {
                *--p = (hcore_uchar_t)(ui64 % 10 + '0');
            } while (ui64 /= 10);
        }
    }
    else if (hexadecimal == 1)
    {
        do
        {
            /* the "(uint32_t)" cast disables the BCC's warning */
            *--p = hex[(hcore_uint32_t)(ui64 & 0xf)];

        } while (ui64 >>= 4);
    }
    else
    { /* hexadecimal == 2 */

        do
        {
            /* the "(uint32_t)" cast disables the BCC's warning */
            *--p = HEX[(hcore_uint32_t)(ui64 & 0xf)];

        } while (ui64 >>= 4);
    }

    /* zero or space padding */

    len = (temp + HCORE_INT64_LEN) - p;

    while (len++ < width && buf < last)
    {
        *buf++ = zero;
    }

    /* number safe copy */

    len = (temp + HCORE_INT64_LEN) - p;

    if (buf + len > last)
    {
        len = last - buf;
    }

    return hcore_cpymem(buf, p, len);
}

hcore_uchar_t *
hcore_slprintf(hcore_uchar_t *buf, hcore_uchar_t *last, const char *fmt, ...)
{
    u_char *p;
    va_list args;

    va_start(args, fmt);
    p = hcore_vslprintf(buf, last, fmt, args);
    va_end(args);

    return p;
}

hcore_uchar_t *
hcore_snprintf(hcore_uchar_t *buf, size_t max, const char *fmt, ...)
{
    u_char *p;
    va_list args;

    va_start(args, fmt);
    p = hcore_vslprintf(buf, buf + max, fmt, args);
    va_end(args);

    return p;
}

hcore_array_t *
hcore_strtokz(hcore_pool_t *pool, hcore_str_t *src, char *delim)
{
    hcore_array_t *t;
    hcore_str_t   *str;
    hcore_uint_t   i;

    t = hcore_strtok(pool, src, delim);

    if (t == NULL) return NULL;

    str = t->elts;

    // copy data to new space and append '\0'
    for (i = 0; i < t->nelts; i++)
    {
        hcore_uchar_t *p = hcore_palloc(pool, str[i].len + 1);
        if (p == NULL) return NULL;

        hcore_memcpy(p, str[i].data, str[i].len);
        p[str[i].len] = 0;

        str[i].data = p;
    }

    return t;
}

hcore_array_t *
hcore_strtok(hcore_pool_t *pool, hcore_str_t *src, char *delim)
{
    hcore_array_t *t;
    hcore_str_t   *str;
    hcore_bool_t   start, sep;
    hcore_uchar_t *p, *last, *p_delim;


    if (src->len == 0)
    {
        return NULL;
    }

    t = hcore_array_create(pool, 1, sizeof(hcore_str_t));
    if (t == NULL)
    {
        return NULL;
    }

    if (delim == NULL)
    {
        delim = " ,\t\r\n";
    }

    last  = src->data + src->len;
    start = 0;
    sep   = 0;
    str   = NULL;

    for (p = src->data; p < last; p++)
    {
        for (p_delim = (u_char *)delim; *p_delim; p_delim++)
        {
            if (*p == *p_delim)
            {
                sep = 1; // found a seperator
                break;
            }
        }

        if (sep)
        {
            sep   = 0;
            start = 0;
            continue; // next character
        }
        else if (start == 0)
        {
            start = 1; // start of substring

            str = hcore_array_push(t);
            if (str == NULL)
            {
                return NULL;
            }

            str->data = p;
            str->len  = 0;
        }

        hcore_assert(str);

        if (str)
        {
            str->len++;
        }
        else
        {
            return NULL;
        }
    }

    return t;
}

char *
hcore_strcpyd(hcore_pool_t *pool, const char *string)
{
    size_t len = strlen(string);

    char *dst = hcore_pnalloc(pool, len + 1);

    if (dst == NULL) return NULL;

    hcore_memcpy(dst, string, len + 1);

    return dst;
}
