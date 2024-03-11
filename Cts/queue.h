/*
 * CTS_QUEUE_H
 *
 * CtsQueue is a queue implementation based on a singly linked list. It provides a standard
 * queue interface with operations such as checking if the queue is empty, getting the size of 
 * the queue, peeking at the front of the queue, enqueuing (pushing) and dequeuing (popping) elements,
 * and freeing up the queue.
 *
 * The queue can store pointers to any type of data. The management and destruction of this data 
 * is a responsibility of the user of the queue, except for the destruction during the full clean up 
 * of the queue which is handled by a user-supplied function. 
 *
 * To clean up a CtsQueue completely, including the stored elements, use cts_queue_free_full().
 *
 * Example usage:
 *
 * // Initialize default allocator
 * cts_allocator_init_default();
 * CtsAllocator* alloc = cts_allocator_get_default();
 *
 * // Create a new queue
 * CtsQueue* queue = cts_queue_new(alloc);
 *
 * // Enqueue some data
 * int* data = cts_allocator_alloc(alloc, sizeof(int));
 * *data = 42;
 * cts_queue_enqueue(queue, data);
 *
 * // Peek at the front of the queue
 * int* peeked_data = (int*)cts_queue_peek(queue);
 * printf("Front of Queue: %d\n", *peeked_data); // Front of Queue: 42
 *
 * // Dequeue data
 * int* dequeued_data = (int*)cts_queue_dequeue(queue);
 * printf("Dequeued: %d\n", *dequeued_data); // Dequeued: 42
 * cts_allocator_free(alloc, dequeued_data); // Clean up the dequeued data
 *
 * // Clean up
 * cts_queue_free_full(queue, alloc, (SListFreeFunc)cts_allocator_free);
 *
 */

#ifndef CTS_QUEUE_H
#define CTS_QUEUE_H

#include "slist.h"

CTS_BEGIN_DECLARE_TYPE(CtsSList, CtsQueue, cts_queue)
CTS_END_DECLARE_TYPE(CtsQueue, cts_queue)

bool cts_queue_is_empty(CtsQueue* self);
size_t cts_queue_size(CtsQueue* self);
void* cts_queue_peek(CtsQueue* self);
bool cts_queue_enqueue(CtsQueue* self, void* data);
void* cts_queue_dequeue(CtsQueue* self);
void cts_queue_free_full(CtsQueue* stack, cts_pointer alloc, SListFreeFunc func);

#endif // CTS_QUEUE_H
