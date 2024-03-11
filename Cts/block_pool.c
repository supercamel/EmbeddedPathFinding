#include "block_pool.h"
#include <stdio.h>

typedef struct Block {
    struct Block* next;
} Block; 

typedef struct Buffer {
    struct Buffer* next;
    uint8_t data[];
} Buffer;

CtsBlockPool* cts_block_pool_new(CtsAllocator* alloc, size_t blockSize, size_t growSize) {
    CtsBlockPool* pool = cts_allocator_alloc(alloc, sizeof(CtsBlockPool));
    if(pool == NULL) 
    {
        return NULL;
    }
    pool->head = NULL;
    pool->buffers = NULL;
    pool->alloc = alloc;
    pool->buffered_blocks = growSize;
    pool->grow_size = growSize;
    pool->block_size = blockSize > sizeof(Block) ? blockSize : sizeof(Block);
    return pool;
}

void* cts_block_pool_alloc(CtsBlockPool* pool) {
    if (pool->head) {
        Block* block = pool->head;
        pool->head = block->next;
        return block;
    }

    if (pool->buffered_blocks >= pool->grow_size) {
        size_t bufferSize = sizeof(Buffer) + pool->block_size * pool->grow_size;
        Buffer* buffer = cts_allocator_alloc(pool->alloc, bufferSize);
        if(buffer == NULL) 
        {
            return NULL;
        }
        buffer->next = pool->buffers;
        pool->buffers = buffer;
        pool->buffered_blocks = 0;
    }

    uint8_t* data = pool->buffers->data + pool->block_size * pool->buffered_blocks;
    pool->buffered_blocks++;
    return data;
}

void cts_block_pool_free(CtsBlockPool* pool, void* pointer) {
    Block* block = (Block*)pointer;
    block->next = pool->head;
    pool->head = block;
}

void cts_block_pool_delete(CtsBlockPool* pool) {
    cts_block_pool_clear(pool);
    cts_allocator_free(pool->alloc, pool);
}

void cts_block_pool_clear(CtsBlockPool* pool) 
{
    Buffer* buffer = pool->buffers;
    while (buffer) {
        Buffer* next = buffer->next;
        cts_allocator_free(pool->alloc, buffer);
        buffer = next;
    }
    pool->buffered_blocks = pool->grow_size;
    pool->buffers = NULL;
    pool->head = NULL;
}

