/**
 * \file
 * \brief Header file for CtsStack.
 *
 * CtsStack is a stack data structure that provides functions to push, pop, peek and check its size. 
 * The CtsStack structure is built on top of CtsSList and can utilize the CtsBlockPool for efficient memory management.
 * 
 * The CtsStack struct needs to be allocated with a CtsAllocator or CtsBlockPool, which is used for allocating the stack's internal structure.
 * The CtsStack struct should be deallocated by calling cts_stack_free_full when it's no longer needed.
 * 
 * Here's an example of how to use CtsStack with CtsBlockPool:
 * \code
 * cts_allocator_init_default();
 * CtsAllocator* alloc = cts_allocator_get_default();
 * CtsBlockPool* pool = cts_block_pool_new(alloc, sizeof(int), 20);
 * CtsStack* stack = cts_stack_new(alloc);
 *
 * // Push elements
 * int* num = cts_block_pool_alloc(pool);
 * *num = 10;
 * cts_stack_push(stack, num);
 *
 * // Pop an element
 * int *element = (int *)cts_stack_pop(stack);
 * printf("Popped: %d\n", *element);
 *
 * // Clean up
 * cts_stack_free_full(stack, pool, cts_block_pool_free);
 * cts_block_pool_delete(pool);
 * cts_stack_unref(stack);
 * \endcode
 */

#ifndef CST_STACK_H
#define CST_STACK_H

#include "slist.h"

typedef bool (*StackEqualFunc)(const void* obj1, const void* obj2);

CTS_BEGIN_DECLARE_TYPE(CtsSList, CtsStack, cts_stack)
CTS_END_DECLARE_TYPE(CtsStack, cts_stack)

bool cts_stack_push(CtsStack* stack, void* obj);
void* cts_stack_pop(CtsStack* stack);
void* cts_stack_peek(CtsStack* stack);
bool cts_stack_is_empty(CtsStack* stack);
size_t cts_stack_size(CtsStack* stack);
void cts_stack_free_full(CtsStack* stack, cts_pointer alloc, SListFreeFunc func);
CtsStack* cts_stack_copy(CtsStack* stack);
bool cts_stack_contains(CtsStack* stack, void* obj, StackEqualFunc equal);

#endif
