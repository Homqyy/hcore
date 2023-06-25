/**
 * @file hcore_inet.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief 提供处理网络结构和报文的接口
 * @version 0.1
 * @date 2021-09-26
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#ifndef _HCORE_INET_H_INCLUDED_
#define _HCORE_INET_H_INCLUDED_

#include <hcore_constant.h>
#include <hcore_string.h>
#include <hcore_debug.h>
#include <hcore_types.h>

#include <arpa/inet.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>

#define HCORE_INET_ADDRSTRLEN    (sizeof("255.255.255.255") - 1)
#define HCORE_MAC_ADDRESS_STRLEN (sizeof("XX:XX:XX:XX:XX:XX") - 1)

#define HCORE_INET6_ADDRSTRLEN \
    (sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255") - 1)

#define HCORE_UNIX_ADDRSTRLEN                           \
    (sizeof("unix:") - 1 + sizeof(struct sockaddr_un) \
     - offsetof(struct sockaddr_un, sun_path))
#define HCORE_MAC_ADDRESS_STRLEN    (sizeof("XX:XX:XX:XX:XX:XX") - 1)


typedef union
{
    struct sockaddr     sockaddr;
    struct sockaddr_in  sockaddr_in;
    struct sockaddr_in6 sockaddr_in6;
    struct sockaddr_un  sockaddr_un;
} hcore_sockaddr_t;

typedef struct
{
    int             family;
    struct sockaddr sockaddr;
    socklen_t       socklen;

    in_port_t port;
    hcore_str_t port_text;
    hcore_str_t host;
    hcore_str_t url_text; // 值为 'hcore_parse_url()' 接口的 'str' 参数

    char *err;

    hcore_uint_t listen : 1;
} hcore_url_t;

/**
 * @brief  将 'struct sockaddr' 转化为字符串格式，存储到 'text' 中
 * @note
 * @param  sa : 套接字结构体，比如：'struct sockaddr_in'
 * @param  socklen: 结构体大小，比如：sizeof(struct sockaddr_in)
 * @param  text : 存储转化后的字符串的地方
 * @param  len: 'text' 的长度
 * @retval
 */
size_t hcore_sock_ntop(struct sockaddr *sa, socklen_t socklen, hcore_uchar_t *text,
                     size_t len);

/**
 * @brief  解析URL字符串，当前支持以下几种格式：
 * 1. "ip:port"，比如：127.0.0.1:60000
 * 2. "unix:path"，比如：unix:/run/tcp.sock
 * @note
 * @param  str: 待解析的字符串
 * @param  listener: 这是一个出参，将解析后的结果放置到此结构中
 * @retval
 */
hcore_int_t hcore_parse_url(const char *str, hcore_url_t *url);

/**
 * @brief  解析IP字符串为 in_addr_t 结构
 * @note
 * @param  text: 待解析的IP字符串
 * @param  len: 'text' 的长度
 * @retval
 */
in_addr_t hcore_inet_addr(hcore_uchar_t *text, size_t len);

static inline size_t
hcore_get_max_addr_len(int family)
{
    switch (family)
    {
    case AF_INET: return HCORE_INET_ADDRSTRLEN;
    case AF_INET6: return HCORE_INET6_ADDRSTRLEN;
    case AF_UNIX: return HCORE_UNIX_ADDRSTRLEN;
    default: return HCORE_UNIX_ADDRSTRLEN;
    }
}

static inline hcore_int_t
hcore_parse_listen(const char *str, hcore_url_t *url)
{
    hcore_assert(str && url);

    url->listen = 1;
    return hcore_parse_url(str, url);
}

// 32-bit swap
unsigned int hcore_swap32(unsigned int value);

// Endian conversion 32bit
unsigned int hcore_endian32(unsigned int src);

/*
 * format: you can set range port with 'Port1-Port2' or set single port with 'Port' or 'Port-Port', such as
 *     range  : 5000-5010
 *     single : 5000
 *     single : 5000-5000
 */
hcore_int_t hcore_parse_port_range(hcore_int_t *port_begin, hcore_int_t *port_end,
                               hcore_uchar_t *data, size_t len);

/*
 * format: IP/MASK
 */
hcore_int_t hcore_parse_ip_and_mask(in_addr_t *ip, in_addr_t *mask,
                                hcore_uchar_t *data, size_t len);

/*
 * format: MAC/MASK
 */
hcore_int_t hcore_parse_mac_and_mask(hcore_uchar_t mac[6], hcore_uchar_t mask[6],
                                 hcore_uchar_t *data, size_t len);

/*
 * format1: XX:XX:XX:XX:XX:XX
 * format2: XX-XX-XX-XX-XX-XX
 */
hcore_int_t hcore_parse_mac(hcore_uchar_t mac[6], hcore_uchar_t *data, size_t len);

/*
 * checking 'bytes' is according as mask. such as
 *     1. valid mask for one byte: 11110000
 *     2. invalid mask for one byte: 11010000
 * assume example 1 then the function will return NGX_OK.
 * assume example 2 then the function will return NGX_ERROR.
 */
hcore_int_t hcore_check_mask(void *data, size_t len);

#endif // !_HCORE_INET_H_INCLUDED_
