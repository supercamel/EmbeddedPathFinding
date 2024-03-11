#include "stack.h"
#include "slist.h"

CTS_DEFINE_TYPE(CtsSList, cts_slist, CtsStack, cts_stack)


bool cts_stack_construct(CtsStack* self)
{
    (void)(self);
    return true;
}

void cts_stack_destruct(CtsStack* self) { 
    (void)(self);
}


bool cts_stack_push(CtsStack* stack, void* obj)
{
    CtsSList* list = (CtsSList*)stack;
    return cts_slist_prepend(list, obj);
}

void* cts_stack_pop(CtsStack* stack)
{
    CtsSList* list = (CtsSList*)stack;
    return cts_slist_remove(list, 0);
}

void* cts_stack_peek(CtsStack* stack)
{
    CtsSList* list = (CtsSList*)stack;
    return cts_slist_get(list, 0);
}

bool cts_stack_is_empty(CtsStack* stack)
{
    CtsSList* list = (CtsSList*)stack;
    return cts_slist_get_length(list) == 0;
}
size_t cts_stack_size(CtsStack* stack)
{
    CtsSList* list = (CtsSList*)stack;
    return cts_slist_get_length(list);
}

void cts_stack_free_full(CtsStack* stack, cts_pointer alloc, SListFreeFunc free_func)
{
    cts_slist_free_full((CtsSList*)stack, alloc, free_func);
}

CtsStack* cts_stack_copy(CtsStack* stack)
{
    CtsStack* copy = cts_stack_new(cts_base_get_allocator((CtsBase*)stack));
    if(!copy) {
        return NULL;
    }

    CtsSListIterator *iter = cts_slist_iterator_new_from_list(cts_base_get_allocator((CtsBase*)stack), (CtsSList*)stack);
    if(!iter) {
        cts_stack_unref(copy);
        return NULL;
    }
    while(cts_slist_iterator_has_next(iter)) {
        cts_stack_push(copy, cts_slist_iterator_next(iter));
    }
    cts_slist_reverse((CtsSList*)copy);
    return NULL;
}

bool cts_stack_contains(CtsStack* stack, void* obj, StackEqualFunc equal)
{
    CtsSListIterator* iter = cts_slist_iterator_new_from_list(cts_base_get_allocator((CtsBase*)stack), (CtsSList*)stack);
    while(cts_slist_iterator_has_next(iter)) {
        if(equal(obj, cts_slist_iterator_next(iter))) {
            return true;
        }
    }
    return false;
}

