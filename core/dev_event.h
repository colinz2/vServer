#ifndef _DEV_EVENT_h
#define _DEV_EVENT_h 
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/epoll.h>

typedef enum {
    EVENT_READ = 1,
    EVENT_WRITE = 2,
    EVENT_ERR = 4,
} dev_ev_type_t;

typedef int  (*loop_cb_t)(void *, uint32_t);
typedef int  (*handler_t)(void *);

typedef struct _dev_event_t 
{
    int fd;
    uint32_t  events;
    handler_t handler;
    void *data;
    char priv[0];
} dev_event_t;

typedef struct _dev_ev_loop
{
    int ep_fd;
    int ev_max;
    struct epoll_event *ep_events;
    loop_cb_t cb;
} dev_event_loop_t;

dev_event_loop_t *dev_event_loop_creat(int max_event, loop_cb_t cb);
void dev_event_loop_destory(dev_event_loop_t * loop);


static inline void* dev_event_get_priv(dev_event_t *event_ptr)
{
    return event_ptr->priv;
}
static inline void* dev_event_get_data(dev_event_t *event_ptr)
{
    return event_ptr->data;
}
static inline int dev_event_get_fd(dev_event_t *event_ptr)
{
    return event_ptr->fd;
}

int dev_event_loop_run(dev_event_loop_t* loop);
int dev_event_loop_add(dev_event_loop_t* loop, dev_event_t *event_ptr);
int dev_event_loop_remove(dev_event_loop_t* loop, dev_event_t *event_ptr);

dev_event_t* dev_event_creat(int fd, uint32_t events, handler_t handler, void *data, int priv_len);
void dev_event_destory(dev_event_t* ev);

#define DEV_DECL_PRIV(event_ptr, priv) priv_data_t* priv = (priv_data_t*)(dev_event_get_priv(event_ptr))
#define DEV_DECL_FD(event_ptr, fd)   int fd = (int)(dev_event_get_fd(event_ptr))
#define DEV_DECL_DATA(event_ptr, type, data)   (type *) data = (type *)(dev_event_get_data(event_ptr))


#define Print(fmt, args...) \
        do { \
            fprintf(stderr, "DBG:%s(%d)-%s: "fmt"\n", __FILE__, __LINE__, __FUNCTION__, ##args); \
            fflush(stderr); \
        } while (0)
#endif
