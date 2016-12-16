#include "dev_event.h"
#include "dev_event_timer.h"
#include "dev_heap.h"
#include <sys/timerfd.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define ONE_SECOND  1000000000
#define ONE_MSECOND 1000000
#define ONE_VSECOND 1000

typedef struct _priv_date_t
{
    dev_heap_t *tm_heap;
    struct timespec ts_curr;
} priv_data_t;

static inline int 
timespec_cmp(struct timespec *ts1, struct timespec *ts2)
{
    if (ts1->tv_sec > ts2->tv_sec) {
        return 1;
    }
    else if (ts1->tv_sec == ts2->tv_sec) {
        if (ts1->tv_nsec > ts2->tv_nsec) {
            return 1;
        }
        else if (ts1->tv_nsec == ts2->tv_nsec) {
            return 0;
        }
        else {
            return -1;
        }
    }
    else {
        return -1;
    }
}

static inline int
get_current_timespec(struct timespec *curr)
{
   return clock_gettime(CLOCK_MONOTONIC, curr);
}

static inline struct timespec 
get_it_timespec(double timeout) 
{
    long long int sec = (long long int)timeout;
    long long int nsec = (long long int)((timeout - (double)sec) * ONE_SECOND);
    struct timespec ts;

    ts.tv_sec = sec;
    ts.tv_nsec = nsec;
    return ts;
}

static inline struct timespec
get_it_timespec_timeout(double timeout) 
{
    long long int sec = (long long int)timeout;
    long long int nsec = (long long int)((timeout - (double)sec) * ONE_SECOND);
    struct timespec ts;

    get_current_timespec(&ts);
    ts.tv_sec += sec;
    ts.tv_nsec += nsec;
    if (ts.tv_nsec >= ONE_SECOND) {
        ts.tv_nsec %= ONE_SECOND;
        ts.tv_sec++;
    }
    return ts;
}

static inline struct timespec 
dec_timespec_minus(struct timespec *tsb, struct timespec *tss) 
{
    struct timespec ts;

    ts.tv_sec = 0;
    ts.tv_nsec = ONE_MSECOND;
    if (timespec_cmp(tsb, tss) > 0) {
        if (tss->tv_nsec > tsb->tv_nsec) {
            tsb->tv_sec--;
            tsb->tv_nsec += ONE_SECOND;
        }
        if (tss->tv_sec > tsb->tv_sec) {
            ts.tv_sec = 0;
            ts.tv_nsec = ONE_MSECOND;
        } else {
            ts.tv_sec = tsb->tv_sec - tss->tv_sec;
            ts.tv_nsec = tsb->tv_nsec - tss->tv_nsec; 
        }
    } 
    return ts;
}

static inline int
dev_timerfd_relative_set(int fd, struct itimerspec *newValue)
{
    return timerfd_settime(fd, 0, newValue, NULL);
}

static inline int 
dev_set_relative_timerfd(int fd, double it_timeout, double interval_timeout)
{
    struct itimerspec newValue;
    memset(&newValue, 0, sizeof(newValue));
    newValue.it_value = get_it_timespec(it_timeout);
    newValue.it_interval = get_it_timespec(interval_timeout);
    if (dev_timerfd_relative_set(fd, &newValue) != 0) {
        fprintf(stderr, "ERROR: timerfd_settime\n");
        return -1;
    }
    return 0;
}

static inline struct itimerspec *
set_it_itimerspec(struct itimerspec *spec, double it_timeout, double interval_timeout) 
{
    spec->it_value = get_it_timespec(it_timeout);
    spec->it_interval = get_it_timespec(interval_timeout);
    return spec;
}

static inline int 
dev_event_timer_cmp_l(void *ev1, void *ev2)
{
    int ret = 0;
    ret = timespec_cmp(&((dev_timer_ev_t *)ev1)->ts, &((dev_timer_ev_t *)ev2)->ts);
    if (ret >= 0) {
        return 0;
    }
    return 1;
}

static inline int 
dev_event_timer_handler(void *ptr)
{
    struct itimerspec newValue;
    void * data = (void *)dev_event_get_data(ptr);
    DEV_DECL_PRIV(ptr, priv);
    DEV_DECL_FD(ptr, fd);

    dev_heap_t * tm_heap = priv->tm_heap;
    dev_timer_ev_t *tm;

    uint64_t exp;
    read(fd, &exp, sizeof(exp));

    while ((tm = dev_heap_get_top(tm_heap))) 
    {
        get_current_timespec(&priv->ts_curr);
        if (timespec_cmp(&priv->ts_curr, &tm->ts) >= 0 ) {
            if (tm->cb != NULL && tm->repeat >= 0) {
                tm->cb(data, tm->ptr);
            }
            tm->ts = get_it_timespec_timeout(tm->timeout);
            if (tm->repeat == 1 || tm->repeat < 0) {
                dev_heap_pop(tm_heap);
            } else if (tm->repeat > 1){
                tm->repeat--;
            }
        } else {
            break;
        }
    }

    if (tm == NULL) {
        dev_set_relative_timerfd(fd, 0, 0);
        return 0;
    }

    get_current_timespec(&priv->ts_curr);

    memset(&newValue, 0, sizeof(newValue));
    newValue.it_value = dec_timespec_minus(&tm->ts, &priv->ts_curr);
    if (dev_timerfd_relative_set(fd, &newValue) != 0) {
        return -1;
    }
    return 0;
}

dev_event_t *
dev_event_timer_creat(int num, void *data)
{
    dev_event_t *ev_ptr;
    int fd;

    if ((fd = timerfd_create(CLOCK_MONOTONIC, 0)) < 0) {
        Print("timerfd_create\n");
        return NULL;
    }
    dev_set_relative_timerfd(fd, 0, 0);
    ev_ptr = dev_event_creat(fd, EPOLLIN /*| DEV_EPOLLET */, dev_event_timer_handler, data, sizeof(priv_data_t));
    if (ev_ptr == NULL) {
        Print("dev_event_creat\n");
        return NULL;
    }
    DEV_DECL_PRIV(ev_ptr, priv);
    priv->tm_heap = dev_heap_creat(num, dev_event_timer_cmp_l);
    if (priv->tm_heap == NULL) {
        return NULL;
    }
    return ev_ptr;
}

int 
dev_event_timer_destory(dev_event_t *ptr)
{
    void * data = (void *)dev_event_get_data(ptr);
    DEV_DECL_PRIV(ptr, priv);
    DEV_DECL_FD(ptr, fd);

    dev_heap_t * tm_heap = priv->tm_heap;

    if (tm_heap) {
        dev_heap_destory(tm_heap);
    }

    if (fd > 0) {
        close(fd);
    }
    dev_event_destory(ptr);
    return 0;
}


int
dev_event_timer_add(dev_event_t *ev, dev_timer_ev_t *tm)
{
    DEV_DECL_PRIV(ev, priv);
    DEV_DECL_FD(ev, fd);
    dev_heap_t * tm_heap = priv->tm_heap;
    dev_timer_ev_t *tm_top;
    struct timespec ts_curr;
    struct itimerspec newValue;

    tm->ts = get_it_timespec_timeout(tm->timeout);

    dev_heap_add(tm_heap, tm);

    tm_top = dev_heap_get_top(tm_heap);

    get_current_timespec(&ts_curr);
    memset(&newValue, 0, sizeof(newValue));
    newValue.it_value = dec_timespec_minus(&tm_top->ts, &ts_curr);
    if (dev_timerfd_relative_set(fd, &newValue) != 0) {
        return -1;
    }

    return 0;
}

dev_timer_ev_t * 
dev_sub_timer_creat(double timeout, char repeat, timer_handler_t handler, void * data)
{
    dev_timer_ev_t *sub_timer = calloc(1, sizeof(dev_timer_ev_t));
    if (sub_timer == NULL) {
        return NULL;
    }

    sub_timer->timeout = timeout;
    sub_timer->cb = handler;
    sub_timer->ptr = data;
    sub_timer->repeat = repeat;

    return sub_timer;
}

void 
dev_sub_timer_remove(dev_timer_ev_t * sub_timer)
{
    sub_timer->repeat = -1;
}

void 
dev_sub_timer_modify_timeout(dev_timer_ev_t *tm, double timeout)
{
    tm->timeout = (double)timeout;
    tm->ts = get_it_timespec_timeout(tm->timeout);
}
