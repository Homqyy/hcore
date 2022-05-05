/**
 * @file hcore_connection.h
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @author homqyy (yilupiaoxuewhq@163.com)
 * @brief 提供连接结构，通过连接接口可以方便的进行事件的处理
 * @version 0.1
 * @date 2021-09-26
 *
 * @copyright Copyright (c) 2021 homqyy
 *
 * @format: UTF-8
 * @abbr:
 */

#ifndef _HCORE_CONNECTION_H_INCLUDED_
#define _HCORE_CONNECTION_H_INCLUDED_

#include <hcore_buf.h>
#include <hcore_connection.h>
#include <hcore_event.h>
#include <hcore_log.h>
#include <hcore_pool.h>
#include <hcore_string.h>

#define HCORE_CHAIN_ERROR (hcore_chain_t *)HCORE_FAILED

typedef struct hcore_connection_s hcore_connection_t;

typedef ssize_t (*hcore_send_pt)(hcore_connection_t *c, hcore_uchar_t *buf,
                               size_t size);

typedef ssize_t (*hcore_recv_pt)(hcore_connection_t *c, hcore_uchar_t *buf,
                               size_t size);

typedef hcore_chain_t *(*hcore_send_chain_pt)(hcore_connection_t *c,
                                          hcore_chain_t *     out);

struct hcore_connection_s
{
    int              fd;        // fd of the connection
    int              type;      // type of socket
    struct sockaddr *sockaddr;  // socket address of peer
    socklen_t        socklen;   // length of sockaddr
    hcore_str_t        addr_text; // text translation of sockaddr

    struct sockaddr *local_sockaddr; // local socket address
    socklen_t        local_socklen;  // local sockaddr length

    hcore_event_t *rev;  // event of read
    hcore_event_t *wev;  // event of write
    hcore_pool_t * pool; // pool of connection
    hcore_log_t *  log;  // log of connection. equal with pool->log

    hcore_send_pt       send;       // send function
    hcore_send_chain_pt send_chain; // send chain function
    hcore_recv_pt       recv;       // receive function

    void *data; // private data

    off_t sent_size; // counted the number of sent byte
    off_t recv_size; // counted the number of received byte

    hcore_uint_t shared  : 1; // to indicate shared fd
    hcore_uint_t pipe    : 1; // is pipe
    hcore_uint_t closed  : 1; // the connection was closed by client
    hcore_uint_t timeout : 1; // timeout for reading or writing
    hcore_uint_t error   : 1; // has a error occur

    /* for udp */
    hcore_uint_t known     : 1; // refuse to receive message from unknown client
    hcore_uint_t bind_peer : 1; // bind peer
};

/**
 * @brief  send 'buf' on udp
 * @note
 * @param  *c: connection
 * @param  *buf: buffer
 * @param  size: size of buffer
 * @retval
 * Upon successful return size of sent. Return HCORE_AGAIN and c->wev->ready will
 * * be set 0 if connection is no ready. Otherwise return HCORE_FAILED (encounter
 * * a error) and c->wev->error will be set 1
 */
ssize_t hcore_udp_send(struct hcore_connection_s *c, hcore_uchar_t *buf, size_t size);

/**
 * @brief  Receive data to 'buf' on udp
 * @note You can control action with flag of below description:
 * 1. set 'c->known = 1' to indicate only receiving data from known client.
 * 2. set 'c->bind_peer = 1' to bind peer to the socket of udp and
 * * 'c->sockaddr', 'c->socklen' will be filled.
 * @param  *c: connection
 * @param  *buf: buffer
 * @param  size: size of buffer
 * @retval
 * Upon successful return size of received. Return HCORE_AGAIN and c->rev->ready
 * * will be set 0 if connection is no ready. Otherwise return HCORE_FAILED
 * * (encounter a error) and c->rev->error will be set 1
 */
ssize_t hcore_udp_recv(struct hcore_connection_s *c, hcore_uchar_t *buf, size_t size);

/**
 * @brief  send 'buf' on tcp
 * @note
 * @param  *c: connection
 * @param  *buf: buffer
 * @param  size: size of buffer
 * @retval
 * Upon successful return size of sent. Return HCORE_AGAIN and c->wev->ready will
 * * be set 0 if connection is no ready. Otherwise return HCORE_FAILED and
 * * c->wev->error will be set 1(encounter a error)
 */
ssize_t hcore_tcp_send(struct hcore_connection_s *c, hcore_uchar_t *buf, size_t size);

/**
 * @brief  Receive data to 'buf' on tcp
 * @note
 * @param  *c: connection
 * @param  *buf: buffer
 * @param  size: size of buffer
 * @retval
 * Upon successful return size of received. Return HCORE_AGAIN and c->rev->ready
 * * will be set 0 if connection is no ready. Otherwise return HCORE_FAILED
 * * (encounter a error) and c->rev->error will be set 1
 */
ssize_t hcore_tcp_recv(struct hcore_connection_s *c, hcore_uchar_t *buf, size_t size);

/**
 * @brief  send 'out' on tcp
 * @note
 * @param  *c: connection
 * @param  *out: buffer chain
 * @retval
 * return 'NULL' if all 'out' was sent.
 * * Otherwise remained chain will be return
 */
hcore_chain_t *hcore_tcp_send_chain(hcore_connection_t *c, hcore_chain_t *out);

#endif // !_HCORE_CONNECTION_H_INCLUDED_
