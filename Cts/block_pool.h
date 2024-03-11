/*
 * BLOCK_POOL_H
 *
 * The CTS Block Pool library provides a special-purpose memory allocation system that 
 * manages blocks of uniform size, known as block pools.
 * This kind of memory management strategy is primarily useful when you require numerous 
 * allocations of the same size, and is especially effective at reducing fragmentation 
 * and improving speed of allocation over generic allocation methods.
 *
 * The Block Pool system has some unique characteristics:
 * 1. Uniform Block Size: All memory blocks allocated from a block pool are the same size.
 * 2. No Overhead: Memory allocation from a block pool incurs no additional memory overhead.
 * 3. Fragmentation Reduction: By allocating in uniform blocks, fragmentation is kept to a minimum.
 * 4. Speed: Block allocation from a block pool is typically faster than generic allocation.
 * 5. Growth: Block pools grow in size automatically as needed. The growth size is set when the pool 
 *    is created, and determines how many blocks will be added when the pool needs to grow.
 * 6. No Shrinkage: Block pools do not shrink automatically, freeing a block doesn't reduce the pool's size.
 *
 * It's important to note that although a block pool doesn't shrink automatically, it can be manually shrunk 
 * by calling the cts_block_pool_delete function, which frees all memory back to its allocator.
 *
 * The function cts_block_pool_new creates a new block pool. The blockSize parameter determines the size 
 * in bytes of each block in the pool. The growSize parameter determines how many blocks to add to the pool 
 * when all current blocks are in use and additional blocks are required.
 *
 * Example usage:
 *
 * // Initialize default allocator
 * cts_allocator_init_default();
 * CtsAllocator* alloc = cts_allocator_get_default();
 * 
 * // Create a new block pool with block size of 64 bytes and growth of 1024 blocks
 * CtsBlockPool* pool = cts_block_pool_new(alloc, 64, 1024);
 * 
 * // Allocate a block from the pool
 * void* block = cts_block_pool_alloc(pool);
 * 
 * // Free a block back to the pool
 * cts_block_pool_free(pool, block);
 * 
 * // Delete the pool when done
 * cts_block_pool_delete(pool);
 *
 */

#ifndef BLOCK_POOL_H
#define BLOCK_POOL_H

#include <stdint.h>
#include <stddef.h>
#include "allocator.h"

typedef struct CtsBlockPool {
    struct Block* head;
    struct Buffer* buffers;
    CtsAllocator* alloc;
    size_t buffered_blocks;
    size_t grow_size;
    size_t block_size;
} CtsBlockPool;

CtsBlockPool* cts_block_pool_new(CtsAllocator* alloc, size_t blockSize, size_t growSize);
void* cts_block_pool_alloc(CtsBlockPool* pool);
void cts_block_pool_free(CtsBlockPool* pool, void* ptr);
void cts_block_pool_delete(CtsBlockPool* pool); //destroys the whole pool and deallocates all memory
void cts_block_pool_clear(CtsBlockPool* pool); //releases all the buffers but keeps the block pool



#endif 

