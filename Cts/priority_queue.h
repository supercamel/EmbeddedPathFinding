/*
 * CtsPriorityQueue is a priority queue implementation using the max heap data structure, provided by the CtsHeap. 
 * A priority queue is a queue where every element has a priority associated with it and the removal is based on the 
 * priority (highest priority is removed first). It is a fundamental abstract data type with diverse applications, 
 * such as managing tasks with different priorities, bandwidth management, Discrete Event Simulation, and many more.
 *
 * The CtsPriorityQueue provides methods to:
 *  - Create a new priority queue with `cts_priority_queue_new_full`.
 *  - Push a new value into the queue with `cts_priority_queue_push`.
 *  - Pop the highest priority value from the queue with `cts_priority_queue_pop`.
 *  - Peek at the highest priority value without removing it with `cts_priority_queue_peek`.
 *  - Check the size of the queue with `cts_priority_queue_get_size`.
 *  - Check if the queue is empty with `cts_priority_queue_is_empty`.
 *  - Clear all elements in the queue with `cts_priority_queue_clear`.
 *
 * All of these operations (except for peek and check operations) manipulate the state of the queue.
 *
 * To create a priority queue, call `cts_priority_queue_new_full`. This function requires a user-provided `HeapCompareFunc` 
 * for comparing elements and a `HeapDestroyFunc` for cleaning up elements when they're removed.
 *
 * Here's an example of how to use CtsPriorityQueue with CtsBlockPool for efficient allocation:
 * \code
 * // Comparison function for integers
 * int compare_ints(cts_pointer a, cts_pointer b) {
 *     return *(int *)b - *(int *)a;
 * }
 *
 * // Initialize default allocator
 * cts_allocator_init_default();
 * CtsAllocator* alloc = cts_allocator_get_default();
 * 
 * // Create a new block pool with block size of sizeof(int) and growth of 100 blocks
 * CtsBlockPool* pool = cts_block_pool_new(alloc, sizeof(int), 100);
 * 
 * // Destruction function for integers using block pool
 * void destroy_int(cts_pointer data) {
 *     cts_block_pool_free(pool, data);
 * }
 *
 * // Create a new priority queue
 * CtsPriorityQueue* queue = cts_priority_queue_new_full(alloc, compare_ints, destroy_int);
 *
 * // Push elements into the queue
 * for (int i = 0; i < 10; i++) {
 *     int* num = cts_block_pool_alloc(pool);
 *     *num = i;
 *     cts_priority_queue_push(queue, num);
 * }
 * 
 * // Pop the highest priority element
 * int* highest_priority = (int*)cts_priority_queue_pop(queue);
 * printf("Highest Priority: %d\n", *highest_priority); // Highest Priority: 9
 *
 * // Clean up
 * cts_priority_queue_clear(queue);
 * cts_block_pool_delete(pool);
 * \endcode
 *
 * Note: This example uses CtsBlockPool for efficient allocation of integers. Block pools are particularly useful when 
 * you need to frequently allocate and deallocate memory blocks of the same size.
 */


#ifndef CTS_PRIORITY_QUEUE_H
#define CTS_PRIORITY_QUEUE_H

#include "object.h"
#include "heap.h"

CTS_BEGIN_DECLARE_TYPE(CtsHeap, CtsPriorityQueue, cts_priority_queue)
CTS_END_DECLARE_TYPE(CtsPriorityQueue, cts_priority_queue)

CtsPriorityQueue* cts_priority_queue_new_full(CtsAllocator* alloc, HeapCompareFunc compare_func, CtsFreeFunc destroy_func, cts_pointer free_ptr);
bool cts_priority_queue_push(CtsPriorityQueue* queue, cts_pointer data);
cts_pointer cts_priority_queue_pop(CtsPriorityQueue* queue);
cts_pointer cts_priority_queue_peek(CtsPriorityQueue* queue);
bool cts_priority_queue_is_empty(CtsPriorityQueue* queue);
size_t cts_priority_queue_get_size(CtsPriorityQueue* queue);
void cts_priority_queue_clear(CtsPriorityQueue* queue);
bool cts_priority_queue_update(CtsPriorityQueue* queue, cts_pointer data);


#endif //CTS_PRIORITY_QUEUE_H
