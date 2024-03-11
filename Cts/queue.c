#include "queue.h"

CTS_DEFINE_TYPE(CtsSList, cts_slist, CtsQueue, cts_queue)


bool cts_queue_construct(CtsQueue* self)
{
    (void)(self);
    return true;
}

void cts_queue_destruct(CtsQueue* self) { 
    (void)(self);
}

bool cts_queue_is_empty(CtsQueue* self)
{
    return cts_slist_get_length((CtsSList*)self) == 0;
}

size_t cts_queue_size(CtsQueue* self)
{
    return cts_slist_get_length((CtsSList*)self);
}

void* cts_queue_peek(CtsQueue* self)
{
    return cts_slist_get((CtsSList*)self, 0);
}

bool cts_queue_enqueue(CtsQueue* self, void* data)
{
    return cts_slist_append((CtsSList*)self, data);
}

void* cts_queue_dequeue(CtsQueue* self)
{
    return cts_slist_remove((CtsSList*)self, 0);
}

void cts_queue_free_full(CtsQueue* stack, cts_pointer alloc, SListFreeFunc func)
{
    cts_slist_free_full((CtsSList*)stack, alloc, func);
}

