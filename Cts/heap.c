#include <string.h>
#include <stddef.h>
#include <limits.h>
#include "heap.h"

CTS_DEFINE_TYPE(CtsBase, cts_base, CtsHeap, cts_heap)

static int default_compare(cts_pointer a, cts_pointer b)
{
    return (a > b) - (a < b);
}

static void cts_heap_heapify(CtsHeap *self, size_t i) {
    size_t largest = i;
    size_t left = 2 * i + 1;
    size_t right = 2 * i + 2;

    // If the left child is larger than the root
    if (left < self->heap_size && self->compare_func(self->arr[left], self->arr[largest]) > 0)
        largest = left;

    // If the right child is larger than the root
    if (right < self->heap_size && self->compare_func(self->arr[right], self->arr[largest]) > 0)
        largest = right;

    // If the root is not the largest
    if (largest != i) {
        // Swap arr[i] and arr[largest]
        cts_pointer temp = self->arr[i];
        self->arr[i] = self->arr[largest];
        self->arr[largest] = temp;

        // Recursively heapify the subtree
        cts_heap_heapify(self, largest);
    }
}

bool cts_heap_construct(CtsHeap *self) {
    CtsAllocator* alloc = cts_base_get_allocator((CtsBase *)self);
    self->arr = cts_allocator_alloc(alloc, 2 * sizeof(cts_pointer));
    memset(self->arr, 0, 2 * sizeof(cts_pointer));
    if(self->arr == NULL) {
        return false;
    }
    self->heap_size = 0;
    self->capacity = 2;
    self->compare_func = default_compare;
    self->destroy_func = (CtsFreeFunc)cts_allocator_free;
    self->free_ptr = (cts_pointer)alloc;
    return true;
}

void cts_heap_destruct(CtsHeap *self) {
    if(self->arr != NULL) {
        CtsAllocator *alloc = cts_base_get_allocator((CtsBase *)self);
        cts_allocator_free(alloc, self->arr);
    }
}

CtsHeap* cts_heap_new_full(CtsAllocator* alloc, HeapCompareFunc compare_func, CtsFreeFunc destroy_func, cts_pointer free_ptr)
{
    CtsHeap* heap = cts_heap_new(alloc);
    if (heap == NULL) {
        return NULL;
    }
    heap->compare_func = compare_func;
    heap->destroy_func = destroy_func;
    heap->free_ptr = free_ptr;
    return heap;
}

bool cts_heap_insert(CtsHeap *self, cts_pointer key) {
    // Check if the heap array needs to be resized
    if (self->heap_size == self->capacity) {
        // Calculate new capacity
        size_t new_capacity = self->capacity * 2;

        // Get the allocator and allocate new array
        CtsAllocator *alloc = cts_base_get_allocator((CtsBase *)self);
        cts_pointer new_arr = cts_allocator_alloc(alloc, new_capacity * sizeof(cts_pointer));

        // Check if memory allocation was successful
        if (new_arr == NULL) {
            return false;
        }

        memset(new_arr, 0, new_capacity * sizeof(cts_pointer));
        // Copy old array to new array and free old array
        memcpy(new_arr, self->arr, self->capacity * sizeof(cts_pointer));
        cts_allocator_free(alloc, self->arr);

        // Update the heap array and capacity
        self->arr = new_arr;
        self->capacity = new_capacity;
    }

    // Add the new key to the heap
    size_t i = self->heap_size++;
    self->arr[i] = key;

    // Move up the heap until the heap property is restored
    while (i != 0 && self->compare_func(self->arr[i], self->arr[(i - 1) / 2]) > 0) {
        // Swap arr[i] and arr[(i - 1) / 2]
        cts_pointer temp = self->arr[i];
        self->arr[i] = self->arr[(i - 1) / 2];
        self->arr[(i - 1) / 2] = temp;

        // Move to the parent
        i = (i - 1) / 2;
    }

    return true;
}

cts_pointer cts_heap_extract_max(CtsHeap *self) {
    if (self->heap_size == 0) {
        return NULL;
    }

    // Get the maximum element (root of the max heap)
    cts_pointer max_item = self->arr[0];

    // Replace the root of the heap with the last element in the heap
    self->arr[0] = self->arr[self->heap_size - 1];

    // Decrease the size of the heap
    self->heap_size--;

    // Restore the heap property for the root element
    cts_heap_heapify(self, 0);

    return max_item;
}

cts_pointer cts_heap_get_max(CtsHeap *self) {
    if (self->heap_size == 0) {
        return NULL;
    }

    // Return the root of the max heap
    return self->arr[0];
}

bool cts_heap_increase_key(CtsHeap *self, size_t i, cts_pointer new_val) {
    if (i >= self->heap_size) {
        return false;
    }

    // Increase the key at index i
    self->arr[i] = new_val;

    // Restore the heap property
    while (i != 0 && self->compare_func(self->arr[i], self->arr[(i - 1) / 2]) > 0) {
        // Swap arr[i] and arr[(i - 1) / 2]
        cts_pointer temp = self->arr[i];
        self->arr[i] = self->arr[(i - 1) / 2];
        self->arr[(i - 1) / 2] = temp;

        // Move to the parent
        i = (i - 1) / 2;
    }

    return true;
}

bool cts_heap_delete_key(CtsHeap *self, size_t i) {
    if (i >= self->heap_size) {
        return false;
    }

    // Call the destroy function to free the key
    self->destroy_func(self->free_ptr, self->arr[i]);

    // Swap key to delete with the last key
    cts_pointer temp = self->arr[i];
    self->arr[i] = self->arr[self->heap_size - 1];
    self->arr[self->heap_size - 1] = temp;

    // Decrease the heap size
    self->heap_size--;

    // Heapify the affected subtree
    cts_heap_heapify(self, i);

    return true;
}



size_t cts_heap_get_size(CtsHeap *self) {
    return self->heap_size;
}

bool cts_heap_is_empty(CtsHeap *self) {
    return self->heap_size == 0;
}

void cts_heap_sort(CtsHeap *self) {
    // Build the max heap
    for (int i = self->heap_size / 2 - 1; i >= 0; i--)
        cts_heap_heapify(self, i);

    // Extract elements from the heap one by one
    for (int i = self->heap_size - 1; i > 0; i--) {
        // Swap arr[0] and arr[i]
        cts_pointer temp = self->arr[0];
        self->arr[0] = self->arr[i];
        self->arr[i] = temp;

        // Heapify the reduced heap
        cts_heap_heapify(self, 0);
    }
}

void cts_heap_free(CtsHeap* self) {
    // Get the allocator
    CtsAllocator *alloc = cts_base_get_allocator((CtsBase *)self);

    // Free all keys
    if(self->destroy_func != NULL)
    {
        for (size_t i = 0; i < self->heap_size; i++) {
            self->destroy_func(self->free_ptr, self->arr[i]);
        }
    }

    // Free the heap array
    cts_allocator_free(alloc, self->arr);

    // Reset the heap
    self->arr = NULL;
    self->heap_size = 0;
    self->capacity = 0;
}
