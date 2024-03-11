#include "priority_queue.h"

CTS_DEFINE_TYPE(CtsHeap, cts_heap, CtsPriorityQueue, cts_priority_queue)

bool cts_priority_queue_construct(CtsPriorityQueue* queue) 
{
    (void)queue;
    return true;
}

void cts_priority_queue_destruct(CtsPriorityQueue* queue)
{
    (void)queue;
}

CtsPriorityQueue* cts_priority_queue_new_full(CtsAllocator* alloc, HeapCompareFunc compare_func, CtsFreeFunc destroy_func, cts_pointer free_ptr)
{
    CtsPriorityQueue* queue = cts_priority_queue_new(alloc);
    if (queue == NULL) {
        return NULL;
    }
    CtsHeap* h = (CtsHeap*)queue;
    h->compare_func = compare_func;
    h->destroy_func = destroy_func;
    h->free_ptr = free_ptr;
    return queue;
}

bool cts_priority_queue_push(CtsPriorityQueue* queue, cts_pointer data)
{
    CtsHeap* h = (CtsHeap*)queue;
    return cts_heap_insert(h, data);
}

cts_pointer cts_priority_queue_pop(CtsPriorityQueue* queue)
{
    CtsHeap* h = (CtsHeap*)queue;
    return cts_heap_extract_max(h);
}

cts_pointer cts_priority_queue_peek(CtsPriorityQueue* queue)
{
    CtsHeap* h = (CtsHeap*)queue;
    return cts_heap_get_max(h);
}

bool cts_priority_queue_is_empty(CtsPriorityQueue* queue)
{
    CtsHeap* h = (CtsHeap*)queue;
    return cts_heap_is_empty(h);
}

size_t cts_priority_queue_get_size(CtsPriorityQueue* queue)
{
    CtsHeap* h = (CtsHeap*)queue;
    return cts_heap_get_size(h);
}

void cts_priority_queue_clear(CtsPriorityQueue* queue)
{
    CtsHeap* h = (CtsHeap*)queue;
    cts_heap_free(h);
}

bool cts_priority_queue_update(CtsPriorityQueue* queue, cts_pointer data) {
    // Perform a linear scan to find the old item
    for (size_t i = 0; i < cts_heap_get_size((CtsHeap*)queue); i++) {
        CtsHeap* heap = (CtsHeap*)queue;
        if ((heap->arr[i]) == data) {
            // Found the item to update. Remove it from the heap.
            cts_heap_delete_key((CtsHeap*)queue, i);
            // Now add the new item. Note that this might fail if the heap is full.
            return cts_priority_queue_push(queue, data);
        }
    }

    // If we got here, the old item wasn't found in the heap.
    return false;
}
