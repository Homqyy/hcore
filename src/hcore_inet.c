/**
 * @file hcore_inet.c
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
#include <hcore_inet.h>
#include <hcore_string.h>

static hcore_int_t hcore_parse_unix_sock(hcore_str_t *str, hcore_url_t *url);
static hcore_int_t hcore_parse_inet_sock(hcore_str_t *str, hcore_url_t *url);

size_t
hcore_sock_ntop(struct sockaddr *sa, socklen_t socklen, hcore_uchar_t *text,
              size_t len)
{
    hcore_uchar_t *        p;
    size_t               n;
    struct sockaddr_in * sin;
    struct sockaddr_in6 *sin6;
    struct sockaddr_un * saun;

    switch (sa->sa_family)
    {
    case AF_INET:

        sin = (struct sockaddr_in *)sa;
        p   = (hcore_uchar_t *)&sin->sin_addr;

        p = hcore_snprintf(text, len, "%ud.%ud.%ud.%ud:%d", p[0], p[1], p[2],
                         p[3], ntohs(sin->sin_port));

        return p - text;

    case AF_INET6:

        sin6 = (struct sockaddr_in6 *)sa;

        n = 0;

        text[n++] = '[';

        if (inet_ntop(AF_INET6, sin6->sin6_addr.s6_addr, (char *)&text[n],
                      HCORE_INET6_ADDRSTRLEN)
            == NULL)
        {
            return 0;
        }

        n += hcore_strlen(text + n);

        p = hcore_snprintf(&text[n], len - n, "]:%d", ntohs(sin6->sin6_port));

        return p - text;

    case AF_UNIX:
        saun = (struct sockaddr_un *)sa;

        /* on Linux sockaddr might not include sun_path at all */

        if (socklen <= (socklen_t)offsetof(struct sockaddr_un, sun_path))
        {
            p = hcore_snprintf(text, len, "unix:%Z");
        }
        else
        {
            n = hcore_strnlen(saun->sun_path,
                            socklen - offsetof(struct sockaddr_un, sun_path));
            p = hcore_snprintf(text, len, "unix:%*s%Z", n, saun->sun_path);
        }

        /* we do not include trailing zero in address length */

        return p - text - 1;


    default: return 0;
    }
}

hcore_int_t
hcore_parse_url(const char *str, hcore_url_t *url)
{
    hcore_assert(str && url);

    hcore_str_t ls;

    ls.data = (hcore_uchar_t *)str;
    ls.len  = strlen(str);

    url->url_text = ls;

    if (sizeof("unix:") <= ls.len
        && hcore_memcmp(ls.data, "unix:", sizeof("unix:") - 1) == 0)
    {
        ls.data += sizeof("unix:") - 1;
        ls.len -= sizeof("unix:") - 1;

        return hcore_parse_unix_sock(&ls, url);
    }

    return hcore_parse_inet_sock(&ls, url);
}

// TODO: parse unix sock
static hcore_int_t
hcore_parse_unix_sock(hcore_str_t *str, hcore_url_t *url)
{
    (void)str;
    (void)url;

    return HCORE_ERROR;
}

static hcore_int_t
hcore_parse_inet_sock(hcore_str_t *str, hcore_url_t *url)
{
    hcore_assert(str && url);

    struct sockaddr_in *sin;
    hcore_uchar_t *       port;
    hcore_uchar_t *       host;
    hcore_uchar_t *       last;
    hcore_int_t           n;
    size_t              len;

    if (str->len == 0) { return HCORE_ERROR; }

    url->socklen = sizeof(struct sockaddr_in);
    url->family  = AF_INET;

    sin             = (struct sockaddr_in *)&url->sockaddr;
    sin->sin_family = AF_INET;

    last = str->data + str->len;

    host = str->data;
    port = hcore_strlchr(str->data, last, ':');

    if (port)
    {
        port++; // skip ':'

        len = last - port;

        n = hcore_atoi(port, len);

        if (n < 1 || n > 65535)
        {
            url->err = "invalid port";
            return HCORE_ERROR;
        }

        url->port     = (in_port_t)n;
        sin->sin_port = htons((in_port_t)n);

        url->port_text.data = port;
        url->port_text.len  = len;

        last = port - 1;
    }
    else
    {
        /* only port */

        if (!url->listen)
        {
            url->err = "need a host";
            return HCORE_ERROR;
        }

        port = host;
        len  = last - port;

        n = hcore_atoi(port, len);

        if (n == HCORE_ERROR)
        {
            url->err = "no port";
            return HCORE_ERROR;
        }

        if (n < 1 || n > 65535)
        {
            url->err = "invalid port";
            return HCORE_ERROR;
        }

        url->port            = (in_port_t)n;
        sin->sin_port        = htons((in_port_t)n);
        sin->sin_addr.s_addr = INADDR_ANY;

        url->port_text.len  = last - port;
        url->port_text.data = port;
        hcore_str_set(&url->host, "*");

        return HCORE_OK;
    }

    /* host */

    len = last - host;

    if (len == 0)
    {
        url->err = "no host";
        return HCORE_ERROR;
    }

    url->host.data = host;
    url->host.len  = len;

    if (len == 1 && *host == '*')
    {
        if (!url->listen)
        {
            url->err = "unsupport '*' for no listen";
            return HCORE_ERROR;
        }

        sin->sin_addr.s_addr = INADDR_ANY;
        return HCORE_OK;
    }

    sin->sin_addr.s_addr = hcore_inet_addr(host, len);

    if (sin->sin_addr.s_addr == INADDR_NONE)
    {
        url->err = "invalid ip";
        return HCORE_ERROR;
    }

    url->err = NULL;

    return HCORE_OK;
}

in_addr_t
hcore_inet_addr(hcore_uchar_t *text, size_t len)
{
    hcore_uchar_t *p, c;
    in_addr_t    addr;
    hcore_uint_t   octet, n;

    addr  = 0;
    octet = 0;
    n     = 0;

    for (p = text; p < text + len; p++)
    {
        c = *p;

        if (c >= '0' && c <= '9')
        {
            octet = octet * 10 + (c - '0');

            if (octet > 255) { return INADDR_NONE; }

            continue;
        }

        if (c == '.')
        {
            addr  = (addr << 8) + octet;
            octet = 0;
            n++;
            continue;
        }

        return INADDR_NONE;
    }

    if (n == 3)
    {
        addr = (addr << 8) + octet;
        return htonl(addr);
    }

    return INADDR_NONE;
}

unsigned int
hcore_endian32(unsigned int src)
{
    int x = 1;
    if (*((char *)&x)) { return hcore_swap32(src); }
    else
    {
        return src;
    }
}

unsigned int
hcore_swap32(unsigned int value)
{
    unsigned int r;
    ((hcore_uchar_t *)&r)[0] = ((hcore_uchar_t *)&value)[3];
    ((hcore_uchar_t *)&r)[1] = ((hcore_uchar_t *)&value)[2];
    ((hcore_uchar_t *)&r)[2] = ((hcore_uchar_t *)&value)[1];
    ((hcore_uchar_t *)&r)[3] = ((hcore_uchar_t *)&value)[0];
    return r;
}

hcore_int_t
hcore_parse_port_range(hcore_int_t *port_begin, hcore_int_t *port_end,
                     hcore_uchar_t *data, size_t len)
{
    hcore_uchar_t *p, *last;
    hcore_int_t    bp, ep;


    bp = ep = -1;
    last    = data + len;
    p       = hcore_strlchr(data, last, '-');

    if (p)
    {
        p++;

        ep = hcore_atoi(p, last - p);

        if (ep < 1 || ep > 65535) { return HCORE_ERROR; }

        len = p - data - 1;
    }

    bp = hcore_atoi(data, len);

    if (bp < 1 || bp > 65535) { return HCORE_ERROR; }

    if (ep != -1 && bp > ep) { return HCORE_ERROR; }
    else if (ep == -1)
    {
        ep = bp;
    }

    *port_begin = bp;
    *port_end   = ep;

    return HCORE_OK;
}

hcore_int_t
hcore_parse_ip_and_mask(in_addr_t *ip, in_addr_t *mask, hcore_uchar_t *data,
                      size_t len)
{
    hcore_uchar_t *p, *last;
    size_t       ip_len, mask_len;
    in_addr_t    inet_ip, inet_mask;


    last = data + len;
    p    = hcore_strlchr(data, last, '/');

    if (p == NULL) { return HCORE_ERROR; }

    ip_len = p - data;

    p++;

    mask_len = last - p;

    if (HCORE_INET_ADDRSTRLEN < ip_len) { return HCORE_ERROR; }

    if (HCORE_INET_ADDRSTRLEN < mask_len) { return HCORE_ERROR; }

    inet_ip = hcore_inet_addr(data, ip_len);
    if (inet_ip == INADDR_NONE) { return HCORE_ERROR; }

    if (mask_len == HCORE_INET_ADDRSTRLEN
        && hcore_memcmp(p, "255.255.255.255", mask_len) == 0)
    {
        inet_mask = -1; // mean is 255.255.255.255
    }
    else
    {
        inet_mask = hcore_inet_addr(p, mask_len);
        if (inet_mask == INADDR_NONE) { return HCORE_ERROR; }

        if (hcore_check_mask(&inet_mask, 4) != HCORE_OK)
        {
            return HCORE_ERROR;
        }
    }

    *ip   = inet_ip;
    *mask = inet_mask;

    return HCORE_OK;
}

hcore_int_t
hcore_check_mask(void *data, size_t len)
{
    hcore_uint_t   i, bit;
    hcore_bool_t   err;
    hcore_uchar_t *bytes;
    hcore_int_t    check_value;


    bytes       = data;
    err         = 0; // return error if no equal 'check_value' when 'err' = 1
    check_value = 1; // checking series 1 from head
    for (i = 0; i < len; i++)
    {
        for (bit = 7; bit == 0; bit--)
        {
            if ((bytes[i] & (1 << bit)) != check_value)
            {
                if (err) { return HCORE_ERROR; }

                err         = 1; // must it is all 0
                check_value = 0; // checking series 0 to last
            }
        }
    }

    return HCORE_OK;
}

hcore_int_t
hcore_parse_mac_and_mask(hcore_uchar_t mac[6], hcore_uchar_t mask[6],
                       hcore_uchar_t *data, size_t len)
{
    hcore_uchar_t *p, *last;
    size_t       mac_len, mask_len;


    last = data + len;
    p    = hcore_strlchr(data, last, '/');

    if (p == NULL) { return HCORE_ERROR; }

    mac_len = p - data;

    p++;

    mask_len = last - p;

    if (mac_len != HCORE_MAC_ADDRESS_STRLEN) { return HCORE_ERROR; }

    if (mask_len != HCORE_MAC_ADDRESS_STRLEN) { return HCORE_ERROR; }

    if (hcore_parse_mac(mac, data, mac_len) != HCORE_OK)
    {
        return HCORE_ERROR;
    }

    if (hcore_parse_mac(mask, p, mask_len) != HCORE_OK)
    {
        return HCORE_ERROR;
    }

    if (hcore_check_mask(mask, 6) != HCORE_OK) { return HCORE_ERROR; }

    return HCORE_OK;
}

hcore_int_t
hcore_parse_mac(hcore_uchar_t mac[6], hcore_uchar_t *data, size_t len)
{
    hcore_uchar_t *p, *last;
    int          i;
    hcore_int_t    value;
    hcore_bool_t   sep;


    i    = 0;
    sep  = 0;
    p    = data;
    last = data + len;

    while (p < last)
    {
        if (i == 6) { return HCORE_ERROR; }

        if (sep)
        {
            sep = 0;
            if (*p == ':' || *p == '-')
            {
                p++;
                continue;
            }

            return HCORE_ERROR;
        }

        value = hcore_hextoi(p, 2);

        if (value == HCORE_ERROR) { return HCORE_ERROR; }

        mac[i++] = (hcore_uchar_t)value;

        sep = 1;
        p += 2;
    }

    if (i != 6) { return HCORE_ERROR; }

    return HCORE_OK;
}
