/*
 * CTS_ARRAY_H
 *
 * The CTS Array library offers dynamic, resizable arrays that facilitate handling and managing data collections.
 * This library introduces the CtsArray type, which is a dynamic structure that expands and shrinks on demand.
 *
 * Key features:
 * 1. Append Elements: Elements can be efficiently added to the end of the array.
 * 2. Insert Elements: Elements can be placed at specific positions within the array.
 * 3. Remove Elements: Elements can be removed based on their index position.
 * 4. Access Elements: Elements can be accessed directly via their index position.
 * 5. Array Sorting: Built-in sort function to order array elements based on a provided comparison function.
 * 6. Length Querying: Ability to quickly return the number of elements within the array.
 *
 * Importantly, CtsArray must be allocated with a CtsAllocator. This allocator is used to manage the memory 
 * required for the array's internal structure. Once the array is no longer needed, it should be deallocated 
 * by calling array_unref().
 *
 * Example usage:
 *
 * // Initialize default allocator
 * cts_allocator_init_default();
 * CtsAllocator* alloc = cts_allocator_get_default();
 * 
 * // Create a new array
 * CtsArray* array = array_new(alloc);
 * 
 * // Allocate and append a new element
 * int* num = cts_allocator_alloc(alloc, sizeof(int));
 * *num = 10;
 * cts_array_append(array, num);
 * 
 * // Retrieve an element
 * int *element = (int *)cts_array_get(array, 0);
 * printf("Element at index 0: %d\n", *element);
 * 
 * // Clean up
 * cts_array_free_full(array, alloc, cts_allocator_free);
 * cts_array_unref(array);
 *
 */

#ifndef CTS_ARRAY_H
#define CTS_ARRAY_H

#include "object.h"

CTS_BEGIN_DECLARE_TYPE(CtsBase, CtsArray, cts_array)
struct CtsArrayPrivate* private;
CTS_END_DECLARE_TYPE(CtsArray, cts_array)

typedef void (*ArrayFreeFunc)(CtsAllocator* alloc, cts_pointer data);
typedef int (*ArrayCompareFunc)(cts_pointer a, cts_pointer b);

bool cts_array_append(CtsArray* self, cts_pointer obj);
bool cts_array_insert(CtsArray* self, size_t index, cts_pointer obj);
cts_pointer cts_array_remove_index(CtsArray* self, size_t index);
cts_pointer cts_array_get(CtsArray* self, size_t index);
cts_pointer cts_array_replace(CtsArray* self, size_t index, cts_pointer obj);
void cts_array_reverse(CtsArray* self);
size_t cts_array_get_length(CtsArray* self);
void cts_array_sort(CtsArray* self, ArrayCompareFunc func);
void cts_array_free(CtsArray* self);
void cts_array_free_full(CtsArray* self, CtsAllocator* alloc, ArrayFreeFunc func);

#endif
