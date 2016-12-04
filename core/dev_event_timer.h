#ifndef _DEV_EVENT_TIMER_H
#define _DEV_EVENT_TIMER_H

#include <sys/timerfd.h>
#include <stdint.h>
#include "dev_event.h"

typedef int  (*timer_handler_t)(void *ev_data, void *self_data);

typedef struct _dev_timer_ev_t 
{
    int16_t repeat;        
    double timeout;
    struct timespec ts;
    timer_handler_t cb; 
    void * ptr;
} dev_timer_ev_t;


dev_event_t *dev_event_timer_creat(int num, void *data);
int dev_event_timer_add(dev_event_t *ev, dev_timer_ev_t *tm);
dev_timer_ev_t * dev_sub_timer_creat(double timeout, char repeat, timer_handler_t handler, void * data);
void dev_sub_timer_remove(dev_timer_ev_t * sub_timer);
void dev_sub_timer_modify_timeout(dev_timer_ev_t *tm, double timeout);

#endif