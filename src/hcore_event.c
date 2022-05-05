/**
 * @file hcore_event.c
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

#include <hcore_connection.h>
#include <hcore_count.h>
#include <hcore_debug.h>
#include <hcore_event.h>

static int       get_nfds(hcore_event_ctx_t *ctx);
static hcore_int_t hcore_event_judge_with_fd(hcore_event_t *event, void *data);
static hcore_int_t hcore_event_judge_with_delete(hcore_event_t *event, void *data);

static hcore_event_t *get_event(hcore_array_t *events,
                              hcore_int_t (*judge_handler)(hcore_event_t *event,
                                                         void *       data),
                              void *data);

#define get_event_by_fd(events, fd) \
    get_event(events, hcore_event_judge_with_fd, &fd)

#define get_event_was_deleted(events) \
    get_event(events, hcore_event_judge_with_delete, NULL)

#define fd_is_existed(events, fd) (!!get_event_by_fd(events, fd))

hcore_event_ctx_t *
hcore_event_create(hcore_pool_t *pool, sigset_t sigmask, long timeout_ms)
{
    hcore_event_ctx_t *event_ctx;
    long             sec, nsec;

    if (pool == NULL || timeout_ms < 0)
    {
        hcore_bug_on();
        return NULL;
    }

    if (hcore_multip_is_overflow_long(timeout_ms, 1000L * 1000L))
    {
        timeout_ms %= 1000;

        if (hcore_multip_is_overflow_long(timeout_ms, 1000L * 1000L))
        {
            hcore_bug_on();
            hcore_log_error(HCORE_LOG_EMERG, pool->log, 0,
                          "timeout value overflow");
            return NULL;
        }

        sec = timeout_ms / 1000;
    }
    else
    {
        sec  = 0;
        nsec = timeout_ms;
    }

    event_ctx = hcore_palloc(pool, sizeof(hcore_event_ctx_t));
    if (event_ctx == NULL)
    {
        hcore_log_error(HCORE_LOG_ERR, pool->log, 0, "failed for malloc event_ctx");
        return NULL;
    }

    event_ctx->sigmask = sigmask;

    FD_ZERO(&event_ctx->rfds);
    FD_ZERO(&event_ctx->wfds);
    event_ctx->nfds = -1;

    event_ctx->revs = hcore_array_create(pool, 1, sizeof(struct hcore_event_s));

    if (event_ctx->revs == NULL)
    {
        hcore_log_error(HCORE_LOG_ERR, pool->log, 0, "failed for creating revs");
        return NULL;
    }


    event_ctx->wevs = hcore_array_create(pool, 1, sizeof(struct hcore_event_s));

    if (event_ctx->wevs == NULL)
    {
        hcore_log_error(HCORE_LOG_ERR, pool->log, 0, "failed for creating wevs");
        return NULL;
    }

    event_ctx->timeout.tv_sec  = sec;
    event_ctx->timeout.tv_nsec = nsec;

    return event_ctx;
}

hcore_int_t
hcore_del_event(hcore_event_ctx_t *ctx, struct hcore_event_s *event, int event_flag)
{
    struct hcore_event_s *found_ev;
    hcore_connection_t *  c;

    if (ctx == NULL || event || NULL)
    {
        hcore_bug_on();
        return HCORE_FAILED;
    }

    hcore_event_flag_assert(event_flag);

    if (!event->active) { return HCORE_SUCCESSED; }

    c = event->data;

    if (event_flag & HCORE_EVENT_READ)
    {
        found_ev = get_event_by_fd(ctx->revs, c->fd);

        if (found_ev)
        {
            found_ev->deleted = 1;
            FD_CLR(c->fd, &ctx->rfds);
            hcore_log_debug(c->log, 0, "delete write event: #%d", c->fd);
        }
    }
    else if (event_flag & HCORE_EVENT_WRITE)
    {
        found_ev = get_event_by_fd(ctx->wevs, c->fd);

        if (found_ev)
        {
            found_ev->deleted = 1;
            FD_CLR(c->fd, &ctx->wfds);
            hcore_log_debug(c->log, 0, "delete write event: #%d", c->fd);
        }
    }
    else
    {
        return HCORE_FAILED;
    }

    ctx->nfds = get_nfds(ctx);

    event->active = 0;

    return HCORE_SUCCESSED;
}

hcore_int_t
hcore_add_event(hcore_event_ctx_t *ctx, struct hcore_event_s *event, int event_flag)
{
    struct hcore_event_s *new_ev;
    hcore_connection_t *  c;

    if (ctx == NULL || event || NULL)
    {
        hcore_bug_on();
        return HCORE_FAILED;
    }

    hcore_event_flag_assert(event_flag);

    if (event->active) { return HCORE_SUCCESSED; }

    c = event->data;

    if (event_flag & HCORE_EVENT_READ)
    {
        if (fd_is_existed(ctx->revs, c->fd))
        {
            hcore_log_debug(c->log, 0, "event was existed: #%d", c->fd);
            return HCORE_FAILED;
        }

        new_ev = get_event_was_deleted(ctx->revs);

        if (new_ev == NULL)
        {
            new_ev = hcore_array_push(ctx->revs);

            if (new_ev == NULL) { return HCORE_FAILED; }
        }

        *new_ev = *event;

        FD_SET(c->fd, &ctx->rfds);

        hcore_log_debug(c->log, 0, "add read event: #%d", c->fd);
    }
    else if (event_flag & HCORE_EVENT_WRITE)
    {
        if (fd_is_existed(ctx->wevs, c->fd))
        {
            hcore_log_debug(c->log, 0, "event was existed: #%d", c->fd);
            return HCORE_FAILED;
        }

        new_ev = get_event_was_deleted(ctx->wevs);

        if (new_ev == NULL)
        {
            new_ev = hcore_array_push(ctx->wevs);

            if (new_ev == NULL) { return HCORE_FAILED; }
        }

        *new_ev = *event;

        FD_SET(c->fd, &ctx->wfds);

        hcore_log_debug(c->log, 0, "add write event: #%d", c->fd);
    }
    else
    {
        return HCORE_FAILED;
    }

    if (ctx->nfds < c->fd + 1) { ctx->nfds = c->fd + 1; }

    event->active = 1;

    return HCORE_SUCCESSED;
}

static hcore_int_t
hcore_event_judge_with_fd(hcore_event_t *event, void *data)
{
    hcore_connection_t *c;
    int *             fd = data;

    if (event->deleted) { return 0; }

    c = event->data;

    if (c->fd == *fd) { return 1; }

    return 0;
}

static hcore_int_t
hcore_event_judge_with_delete(hcore_event_t *event, void *data)
{
    return event->deleted;
}

static hcore_event_t *
get_event(hcore_array_t *events,
          hcore_int_t (*judge_handler)(hcore_event_t *event, void *data),
          void *data)
{
    hcore_event_t *ev;
    hcore_uint_t   i;

    ev = (hcore_event_t *)events->elts;

    for (i = 0; i < events->nelts; i++)
    {
        if (judge_handler(&ev[i], data) == 1) { return &ev[i]; }
    }

    return NULL;
}

static int
get_nfds(hcore_event_ctx_t *ctx)
{
    hcore_event_t *     ev;
    hcore_connection_t *c;
    hcore_uint_t        i;
    int               nfds = -1;

    ev = (hcore_event_t *)ctx->revs->elts;

    for (i = 0; i < ctx->revs->nelts; i++)
    {
        if (ev[i].deleted) continue;

        c = ev[i].data;
        if (nfds < c->fd + 1) { nfds = c->fd + 1; }
    }

    ev = (hcore_event_t *)ctx->wevs->elts;

    for (i = 0; i < ctx->wevs->nelts; i++)
    {
        if (ev[i].deleted) continue;

        c = ev[i].data;
        if (nfds < c->fd + 1) { nfds = c->fd + 1; }
    }

    return nfds;
}