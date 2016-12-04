#ifndef _DEV_HEAP_H
#define _DEV_HEAP_H

typedef int  (*dev_heap_cmp_t)(void *, void *);
typedef void (*dev_heap_upd_t) (void *elem);

typedef struct _dev_heap
{
    void **array;
    int  array_size;
    int  size;
    dev_heap_cmp_t cmp;
    dev_heap_upd_t update;
} dev_heap_t;


dev_heap_t * dev_heap_creat(int size, dev_heap_cmp_t cmp);
void * dev_heap_get_top(dev_heap_t *heap);
void dev_heap_pop(dev_heap_t *heap);
void dev_heap_add(dev_heap_t *heap, void *data);
void dev_heap_destory(dev_heap_t *heap);

#endif