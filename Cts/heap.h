/*
 * CtsHeap is a max heap implementation, a form of a binary tree where the parent nodes are always larger 
 * than or equal to their children. This data structure is important in several efficient graph algorithms 
 * such as Dijkstra's algorithm, and can be used to create priority queues, check if an array is a heap, 
 * heap sort, and more.
 *
 * The CtsHeap provides methods to:
 *  - Insert a new value into the heap with `cts_heap_insert`.
 *  - Extract the maximum value from the heap with `cts_heap_extract_max`. This operation also removes the maximum element.
 *  - Get the maximum value without removing it from the heap with `cts_heap_get_max`.
 *  - Increase the value of a specific key with `cts_heap_increase_key`.
 *  - Delete a key at a given index with `cts_heap_delete_key`.
 *  - Check the size of the heap with `cts_heap_get_size`.
 *  - Check if the heap is empty with `cts_heap_is_empty`.
 *  - Sort the heap with `cts_heap_sort`.
 *  - Free the heap with `cts_heap_free`.
 *
 * All of these operations (except for get and check operations) manipulate the state of the heap.
 *
 * Note: When an element is removed from the heap, it is the client code's responsibility to free the memory of the element if necessary.
 *
 * To create a heap, call `cts_heap_new_full`. This function requires a user-provided `HeapCompareFunc` 
 * for comparing elements and a `HeapDestroyFunc` for cleaning up elements when they're removed.
 *
 * Here's an example of how to use CtsHeap:
 * \code
 * // Comparison function for integers
 * int compare_ints(cts_pointer a, cts_pointer b) {
 *     return *(int *)b - *(int *)a;
 * }
 *
 * // Destruction function for integers
 * void destroy_int(cts_pointer data) {
 *      CtsAllocator* alloc = cts_allocator_get_default();
 *     cts_allocator_free(alloc, data);
 * }
 *
 * // Initialize default allocator
 * cts_allocator_init_default();
 * CtsAllocator* alloc = cts_allocator_get_default();
 * 
 * // Create a new heap
 * CtsHeap* heap = cts_heap_new_full(alloc, compare_ints, destroy_int);
 *
 * // Insert elements into the heap
 * for (int i = 0; i < 10; i++) {
 *     int* num = cts_allocator_alloc(alloc, sizeof(int));
 *     *num = i;
 *     cts_heap_insert(heap, num);
 * }
 * 
 * // Extract the max element
 * int* max_value = (int*)cts_heap_extract_max(heap);
 * printf("Max Value: %d\n", *max_value); // Max Value: 9
 *
 * // Clean up
 * cts_heap_free(heap);
 * \endcode
 */

#ifndef CTS_HEAP_H
#define CTS_HEAP_H

#include "object.h"

typedef int (*HeapCompareFunc)(cts_pointer a, cts_pointer b);

CTS_BEGIN_DECLARE_TYPE(CtsBase, CtsHeap, cts_heap)
cts_pointer* arr;
size_t heap_size;
size_t capacity;
HeapCompareFunc compare_func;
CtsFreeFunc destroy_func;
cts_pointer* free_ptr;
CTS_END_DECLARE_TYPE(CtsHeap, cts_heap)

CtsHeap* cts_heap_new_full(CtsAllocator* alloc, HeapCompareFunc compare_func, CtsFreeFunc destroy_func, cts_pointer user_free_ptr);
bool cts_heap_insert(CtsHeap *self, cts_pointer key); 
cts_pointer cts_heap_extract_max(CtsHeap *self); 
cts_pointer cts_heap_get_max(CtsHeap *self); 
bool cts_heap_increase_key(CtsHeap *self, size_t i, cts_pointer new_val); 
bool cts_heap_delete_key(CtsHeap *self, size_t i); 
size_t cts_heap_get_size(CtsHeap *self); 
bool cts_heap_is_empty(CtsHeap *self); 
void cts_heap_sort(CtsHeap *self); 
void cts_heap_free(CtsHeap* self);

#endif 

