/**
 * @file hcore_connection.c
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
#include <hcore_buf.h>
#include <hcore_connection.h>
#include <hcore_inet.h>
#include <hcore_io.h>

#include <sys/socket.h>
#include <sys/types.h>

// for writev
typedef struct
{
    struct iovec *iovs;
    hcore_uint_t count;
    size_t size;
    hcore_uint_t nalloc;
} hcore_iovec_t;

static hcore_chain_t *hcore_output_chain_to_iovec(hcore_iovec_t *vec,
                                                  hcore_chain_t *out, hcore_log_t *log);

static hcore_chain_t *hcore_update_output_chain(hcore_chain_t *out, size_t sent);

// TODO: create connection
struct hcore_connection_s *
hcore_create_connection(hcore_log_t *log, int fd)
{
    hcore_pool_t *pool;
    hcore_log_t *new_log;
    struct hcore_connection_s *c;
    hcore_event_t *rev;
    hcore_event_t *wev;

    pool = hcore_create_pool(HCORE_POOL_SIZE_DEFAULT, log);
    if (pool == NULL)
    {
        return NULL;
    }

    new_log = hcore_pnalloc(pool, sizeof(hcore_log_t));
    if (new_log == NULL)
    {
        goto failed;
    }

    *new_log = *log;

    pool->log = new_log;

    c = hcore_pnalloc(pool, sizeof(struct hcore_connection_s));
    if (c == NULL)
    {
        goto failed;
    }
    hcore_memzero(c, sizeof(struct hcore_connection_s));

    rev = hcore_pnalloc(pool, sizeof(hcore_event_t));
    if (rev == NULL)
    {
        goto failed;
    }
    hcore_memzero(rev, sizeof(hcore_event_t));
    rev->data = c;

    wev = hcore_pnalloc(pool, sizeof(hcore_event_t));
    if (wev == NULL)
    {
        goto failed;
    }
    hcore_memzero(wev, sizeof(hcore_event_t));
    wev->data = c;

    c->log = new_log;
    c->pool = pool;
    c->fd = fd;
    c->rev = rev;
    c->wev = wev;

    return c;

failed:
    hcore_destroy_pool(pool);
    return NULL;
}

// TODO: destroy connection
void hcore_destroy_connection(struct hcore_connection_s *c)
{
    if (c->shared)
    {
        c->fd = -1;
    }
    else if (c->fd != -1)
    {
        close(c->fd);
    }

    hcore_destroy_pool(c->pool);
}

ssize_t
hcore_udp_send(struct hcore_connection_s *c, hcore_uchar_t *buf, size_t size)
{
    ssize_t n;
    hcore_event_t *wev;
    hcore_err_t err;

    wev = c->wev;

    for (;;)
    {
        n = sendto(c->fd, buf, size, 0, c->sockaddr, c->socklen);

        hcore_log_debug(c->log, 0, "sendto: fd:%d %z of %uz to \"%V\"", c->fd, n,
                        size, &c->addr_text);

        if (n >= 0)
        {
            if ((size_t)n != size)
            {
                wev->error = 1;
                return HCORE_ERROR;
            }

            c->sent_size += n;

            return n;
        }

        err = errno;

        if (err == EAGAIN)
        {
            wev->ready = 0;
            hcore_log_debug(c->log, errno, "sendto() not ready");
            return HCORE_AGAIN;
        }

        if (err != EINTR)
        {
            wev->error = 1;
            hcore_log_error(HCORE_LOG_ALERT, c->log, errno, "sendto() failed");
            return HCORE_ERROR;
        }
    }
}

ssize_t
hcore_udp_recv(struct hcore_connection_s *c, hcore_uchar_t *buf, size_t size)
{
    ssize_t n;
    struct sockaddr sockaddr;
    socklen_t socklen = sizeof(sockaddr);
    hcore_err_t err;

    if (c->bind_peer && c->socklen)
    {
        sockaddr = *c->sockaddr;
        socklen = c->socklen;
    }

    for (;;)
    {
        n = recvfrom(c->fd, buf, size, 0, &sockaddr, &socklen);
        if (0 < n)
        {
            hcore_log_debug(c->log, 0, "recvfrom %z/%uz bytes", n, size);

            if (socklen == 0 && c->known)
            {
                hcore_log_error(HCORE_LOG_NOTICE, c->log, 0,
                                "receive a message from unknown client");
                continue;
            }

            if (c->bind_peer && !c->socklen)
            {
                c->sockaddr = hcore_pcalloc(c->pool, socklen);
                if (c->sockaddr == NULL)
                {
                    hcore_log_error(HCORE_LOG_ALERT, c->log, 0,
                                    "failed for creating sockaddr");
                    continue;
                }
                *c->sockaddr = sockaddr;
                c->socklen = socklen;

                size_t max_len = hcore_get_max_addr_len(c->sockaddr->sa_family);
                c->addr_text.data = hcore_palloc(c->pool, max_len);
                if (c->addr_text.data == NULL)
                {
                    return HCORE_ERROR;
                }

                c->addr_text.len = hcore_sock_ntop(c->sockaddr, c->socklen,
                                                   c->addr_text.data, max_len);

                hcore_log_debug(c->log, 0, "bind udp of peer '%V' is successful",
                                &c->addr_text);
            }

            return n;
        }

        if (n == -1)
        {
            err = errno;
            if (err == EAGAIN)
            {
                c->rev->ready = 0;
                hcore_log_debug(c->log, err, "recvmsg() not ready");
                return HCORE_AGAIN;
            }

            if (err == EINTR)
                continue;

            hcore_log_error(HCORE_LOG_ALERT, c->log, errno, "recvfrom() failed");

            c->rev->error = 1;

            return HCORE_ERROR;
        }

        /* n == 0 */

        c->rev->eof = 1;

        hcore_log_debug(c->log, 0, "connection was be closed by client");
        return HCORE_ERROR;
    }
}

ssize_t
hcore_tcp_send(struct hcore_connection_s *c, hcore_uchar_t *buf, size_t size)
{
    ssize_t n;
    hcore_err_t err;
    hcore_event_t *wev;

    wev = c->rev;

    for (;;)
    {
        n = send(c->fd, buf, size, 0);

        hcore_log_debug(c->log, 0, "send: fd:#%d %z of %uz", c->fd, n, size);

        if (n > 0)
        {
            if (n < (ssize_t)size)
            {
                wev->ready = 0;
            }

            c->sent_size += n;

            return n;
        }

        err = errno;

        if (n == 0)
        {
            hcore_log_error(HCORE_LOG_ALERT, c->log, err, "send() returned zero");
            wev->ready = 0;
            return n;
        }

        if (err == EAGAIN || err == EINTR)
        {
            wev->ready = 0;

            hcore_log_debug(c->log, err, "send() not ready");

            if (err == EAGAIN)
            {
                return HCORE_AGAIN;
            }
        }
        else
        {
            wev->error = 1;
            hcore_log_error(HCORE_LOG_ALERT, c->log, errno, "send() failed");
            return HCORE_ERROR;
        }
    }
}

ssize_t
hcore_tcp_recv(struct hcore_connection_s *c, hcore_uchar_t *buf, size_t size)
{
    ssize_t n;
    hcore_err_t err;
    hcore_event_t *rev;

    rev = c->rev;

    for (;;)
    {
        n = recv(c->fd, buf, size, 0);

        hcore_log_debug(c->log, 0, "recv: fd:#%d %z of %uz", c->fd, n, size);

        if (n == 0)
        {
            rev->ready = 0;
            rev->eof = 1;

            return 0;
        }

        if (n > 0)
            return n;

        err = errno;

        if (err == EAGAIN || err == EINTR)
        {
            rev->ready = 0;

            hcore_log_debug(c->log, err, "recv() not ready");

            if (err == EAGAIN)
                return HCORE_AGAIN;
        }
        else
        {
            rev->error = 1;
            hcore_log_error(HCORE_LOG_ALERT, c->log, errno, "recv() failed");
            return HCORE_ERROR;
        }
    }
}

hcore_chain_t *
hcore_tcp_send_chain(hcore_connection_t *c, hcore_chain_t *out)
{
    hcore_assert(c && out);

    hcore_event_t *wev = c->wev;
    hcore_iovec_t vec;
    ssize_t n;
    struct iovec iovs[HCORE_IOV_MAX];

    if (!wev->ready)
        return out;

    vec.iovs = iovs;
    vec.nalloc = HCORE_IOV_MAX;

    for (;;)
    {
        // convert 'out' chain to iovec
        if (hcore_output_chain_to_iovec(&vec, out, c->log) == HCORE_CHAIN_ERROR)
            return HCORE_CHAIN_ERROR;

        // writev

        n = hcore_writev(c->fd, vec.iovs, vec.count);
        if (n == -1)
        {
            if (errno != EAGAIN)
            {
                c->wev->error = 1;
                hcore_log_error(HCORE_LOG_ALERT, c->log, errno, "writev() failed");
                return HCORE_CHAIN_ERROR;
            }

            return out;
        }

        /* 0 <= n */

        out = hcore_update_output_chain(out, n);

        c->sent_size += n;

        if (vec.size != n)
        {
            wev->ready = 0;
            return out;
        }
    }
}

static hcore_chain_t *
hcore_update_output_chain(hcore_chain_t *out, size_t sent)
{
    size_t size;

    if (sent == 0)
        return out;

    for (/* void */; out; out = out->next)
    {
        size = hcore_buf_get_size(out->buf);

        size = hcore_min(sent, size);

        out->buf->pos += size;

        sent -= size;

        if (sent == 0)
            break;
    }

    return out;
}

static hcore_chain_t *
hcore_output_chain_to_iovec(hcore_iovec_t *vec, hcore_chain_t *out, hcore_log_t *log)
{
    hcore_assert(vec && out && log);

    size_t size, total;
    hcore_uint_t n;
    hcore_uchar_t *prev_last;
    struct iovec *iov;

    n = 0;
    total = 0;
    prev_last = NULL;

    for (/* void */; out; out = out->next)
    {
        hcore_assert(out->buf);

        if (n == vec->nalloc)
            break;

        size = hcore_buf_get_size(out->buf);

        if (prev_last == out->buf->pos)
        {
            // concat buffer
            iov->iov_len += size;
        }
        else
        {
            iov = &vec->iovs[n++];

            iov->iov_base = (void *)out->buf->pos;
            iov->iov_len = size;
        }

        prev_last = out->buf->pos + size;
        total += size;
    }

    vec->count = n;
    vec->size = total;

    return out;
}