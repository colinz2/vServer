#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include "dev_event.h"

dev_event_t* 
dev_event_creat(int fd, uint32_t events, handler_t handler, void *data, int priv_len)
{
    dev_event_t *ev = NULL;
    ev = calloc(1, sizeof(dev_event_t) + priv_len);
    if (ev == NULL) {
        return NULL;
    }
    ev->fd = fd;
    ev->events = events;
    ev->handler = handler;
    ev->data = data;
    return ev;
}

dev_event_loop_t * 
dev_event_loop_creat(int max_event, loop_cb_t cb) 
{
    dev_event_loop_t *loop;
    loop = (dev_event_loop_t *)calloc(1, sizeof(dev_event_loop_t));

    loop->ev_max = max_event;
    loop->ep_fd = epoll_create(max_event);
    if (loop->ep_fd == -1) {
        Print("create epoll:%s\n", strerror(errno));
        return NULL;
    }

    loop->ep_events = (struct epoll_event *)calloc(max_event, sizeof(struct epoll_event));
    if (loop->ep_events == NULL) {
        Print("No enough memory for loop->ep_events\n");
        return NULL;
    }
    loop->cb = cb;
    return loop;
}

int 
dev_event_loop_run(dev_event_loop_t* loop) 
{
    int i, num;

    for(;;) {
        num = epoll_wait(loop->ep_fd, loop->ep_events, loop->ev_max, -1);
        if (num == -1) {
            if (errno == EINTR)
                continue;
            Print("epoll_wait: %s\n", strerror(errno));
            return -1;
        }
        for (i = 0; i < num; i++) {
            struct epoll_event *ev = &loop->ep_events[i];
            uint32_t events = 0;

            if (ev) {
                if (ev->events & EPOLLERR) {
                    events |= EVENT_ERR;
                }

                if (ev->events & (EPOLLIN | EPOLLHUP)) {
                    events |= EVENT_READ;
                }

                if (ev->events & EPOLLOUT) {
                    events |= EVENT_WRITE;
                }
               
                if (ev->data.ptr) {
                    loop->cb(ev->data.ptr, events);
                }
            }
        }
    }
    return 0;
}

int 
dev_event_loop_add(dev_event_loop_t* loop, dev_event_t *event)
{
    struct epoll_event ev;
    int ev_fd;

    if (loop == NULL || event == NULL) {
        return -1;
    } 
    ev.data.ptr = event;
    ev.events  = event->events;
    ev_fd = event->fd;
    if(-1 == epoll_ctl(loop->ep_fd, EPOLL_CTL_ADD, ev_fd, &ev)) {
        Print("epoll_ctl: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

int 
dev_event_loop_remove(dev_event_loop_t* loop, dev_event_t *event)
{
    struct epoll_event ev;
    int fd;

    fd = event->fd;
    ev.data.fd = event->fd;
    ev.events = event->events;
    if (-1 == epoll_ctl(loop->ep_fd, EPOLL_CTL_DEL, fd, &ev)) {
        Print("epoll_ctl: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

int 
dev_event_loop_pause(dev_event_loop_t* loop, dev_event_t *event)
{
    uint32_t ev_type;
    struct epoll_event ev;
    int fd;

    fd = event->fd;
    ev.data.fd = fd;
    ev_type = event->events;
    //if (ev_type & DEV_EPOLLIN) ev_type &= (~DEV_EPOLLIN);
    ev.events = ev_type;
    if (-1 == epoll_ctl(loop->ep_fd, EPOLL_CTL_MOD, fd, &ev)) {
        Print("epoll_ctl: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

