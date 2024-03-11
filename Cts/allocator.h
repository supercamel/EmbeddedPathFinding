#ifndef CTS_ALLOCATOR_H
#define CTS_ALLOCATOR_H

#include <stddef.h>

/**
 * CtsAllocator is required to create objects in the C type system
 * It is basically an interface for any memory allocator, allowing flexibility for custom allocators
 * We offer 2 varieties of allocators, the 'default' allocator and a pool allocator
 * 'default' allocator is a simple wrap around malloc/realloc/free
 * pool allocators are created from a pool of memory. they behave exactly like malloc/realloc/free except they only allocate memory from the pool
*/
typedef struct CtsAllocator
{
    void* (*alloc)(struct CtsAllocator* self, size_t size);
    void* (*realloc)(struct CtsAllocator* self, void* ptr, size_t size);
    void (*free)(struct CtsAllocator* self, void* ptr);
} CtsAllocator;

// the 'default' allocator simply wraps malloc/realloc/free
void cts_allocator_init_default();
CtsAllocator* cts_allocator_get_default();

// creates an allocator that allocates memory from a blob of memory
// this is useful for localising memory allocations and preventing heap fragmentation
CtsAllocator* cts_allocator_from_pool(void* pool, size_t pool_size);

// allocation functions
void* cts_allocator_alloc(CtsAllocator* allocator, size_t size);
void* cts_allocator_realloc(CtsAllocator* allocator, void* ptr, size_t size);
void cts_allocator_free(CtsAllocator* allocator, void* ptr);

#endif

