#include "dev_heap.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define HEAP_LCHLD(i) (i << 1)
#define HEAP_RCHLD(i) ((i << 1) | 1)
#define HEAP_PARNT(i) (i >> 1)
#define HEAP_SIZE(h)  (h->size)


dev_heap_t *
dev_heap_creat(int size, dev_heap_cmp_t cmp)
{
    dev_heap_t *heap;

    heap = calloc(1, sizeof(dev_heap_t));
    if (heap == NULL) {
        return NULL;
    }
    heap->array_size = size;
    heap->size = 0;
    heap->array = calloc(size, sizeof(heap->array));
    if (heap->array == NULL) {
        return NULL;
    }
    heap->cmp = cmp;

    return heap;
}

static inline int
dev_heap_trickle_up(dev_heap_t *heap, int index)
{
    void *tmp = heap->array[index];

    while (index > 1 && heap->cmp(tmp, heap->array[HEAP_PARNT(index)])) {
        heap->array[index] = heap->array[HEAP_PARNT(index)];
        index = HEAP_PARNT(index);
    }
    heap->array[index] = tmp;
    return index;
}

static inline int
dev_heap_tricle_down(dev_heap_t *heap, int index)
{
    void *tmp = heap->array[index];
    int  index_tmp;

    while (HEAP_LCHLD(index) <= HEAP_SIZE(heap)) {
        index_tmp = HEAP_LCHLD(index);

        if (HEAP_RCHLD(index) <= HEAP_SIZE(heap) && heap->cmp(heap->array[index_tmp + 1], heap->array[index_tmp])) {
            index_tmp++;
        } 

        if (heap->cmp(tmp, heap->array[index_tmp])) {
            break;
        } else {
            heap->array[index] = heap->array[index_tmp];
            index = index_tmp;
        }
    }
    heap->array[index] = tmp;
    return index;
}

void *
dev_heap_get_top(dev_heap_t *heap)
{
    dev_heap_tricle_down(heap, 1);
    return heap->array[1];
}

void 
dev_heap_pop(dev_heap_t *heap)
{
    if (HEAP_SIZE(heap) < 1) {
        return ;
    }

    free(heap->array[1]);
    heap->array[1] = heap->array[HEAP_SIZE(heap)];
    heap->array[HEAP_SIZE(heap)] = NULL;
    heap->size--;
    dev_heap_tricle_down(heap, 1);
}

/*static int
dev_heap_expand(dev_heap_t *heap)
{

}
*/

void
dev_heap_add(dev_heap_t *heap, void *data)
{
    heap->size++;
    heap->array[heap->size] = data;
    dev_heap_trickle_up(heap, heap->size);
}

void
dev_heap_destory(dev_heap_t *heap)
{
    free(heap->array);
    free(heap);
}
