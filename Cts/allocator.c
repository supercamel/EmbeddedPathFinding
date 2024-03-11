#include "allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static CtsAllocator cts_default_allocator;

size_t n_allocs = 0;

static void *default_alloc(CtsAllocator *self, size_t size)
{
    (void)(self);
    return malloc(size);
}

static void *default_realloc(CtsAllocator *self, void *ptr, size_t size)
{
    (void)(self);
    return realloc(ptr, size);
}

static void default_free(CtsAllocator *self, void *ptr)
{
    (void)(self);
    free(ptr);
}

void cts_allocator_init_default()
{
    cts_default_allocator.alloc = default_alloc;
    cts_default_allocator.realloc = default_realloc;
    cts_default_allocator.free = default_free;
}

CtsAllocator *cts_allocator_get_default()
{
    return &cts_default_allocator;
}

void *cts_allocator_alloc(CtsAllocator *allocator, size_t size)
{
    void* ptr = allocator->alloc(allocator, size);
    if(ptr != NULL) {
        n_allocs++;
    }
    return ptr;
}

void *cts_allocator_realloc(CtsAllocator *allocator, void *ptr, size_t size)
{
    return allocator->realloc(allocator, ptr, size);
}

void cts_allocator_free(CtsAllocator *allocator, void *ptr)
{
    n_allocs--;
    allocator->free(allocator, ptr);
}

typedef struct AllocatedHead 
{
    uint16_t size;
    uint16_t used;
} AllocatedHead;

typedef struct BlockHead
{
    uint16_t size;
    uint16_t used;
    uint16_t next;
    uint16_t prev;
} BlockHead;

#define CHUNK_SIZE sizeof(BlockHead)

typedef union Block
{
    BlockHead head;        // when block is free
    char data[CHUNK_SIZE]; // when block is occupied
} Block;

typedef struct Pool
{
    CtsAllocator allocator;
    uint16_t free_list_index;
    Block *blocks;
    size_t num_blocks;
} Pool;

static size_t coalesce(Pool *pool);
static int block_is_free(Block *block);
static void join_adjacent(Pool *pool, size_t i);
static void split_block(Pool *pool, Block *n, uint32_t split_pos);
static void *alloc_from_free_list(Pool *pool, size_t size);
static void* pool_alloc(CtsAllocator *pool, size_t size);
static void* pool_realloc(CtsAllocator *pool, void *ptr, size_t size);
static void pool_free(CtsAllocator *pool, void *ptr);


static int block_is_free(Block *block)
{
    return (block->head.used == 0);
}

static void split_block(Pool *pool, Block *n, uint32_t split_pos)
{
    // Calculate the index of the current block 'n' in the pool
    uint16_t n_index = n - pool->blocks;

    // Calculate the index of the new block to be split
    uint16_t split_index = n_index + split_pos;

    // Pointer to the new block
    Block *sp = &pool->blocks[split_index];

    if (pool->free_list_index != UINT16_MAX)
    {
        pool->blocks[pool->free_list_index].head.prev = split_index;
    }

    // Initialize the new block
    sp->head.size = n->head.size - split_pos;
    sp->head.next = pool->free_list_index;
    sp->head.prev = UINT16_MAX;
    sp->head.used = 0; // new block is free

    // Update the free list index to point to the new block
    pool->free_list_index = split_index;

    // Update the size of the original block
    n->head.size = split_pos;
}


static void join_adjacent(Pool *pool, size_t block_n)
{
    uint32_t first = block_n;
    block_n += pool->blocks[block_n].head.size;
    if (block_n >= pool->num_blocks)
    {
        return;
    }

    uint32_t count = block_n;
    while ((block_is_free(&pool->blocks[block_n])) &&
           (count < pool->num_blocks))
    {
        pool->blocks[first].head.size += pool->blocks[block_n].head.size;
        count += pool->blocks[block_n].head.size;

        //if (&pool->blocks[block_n] == pool->free_list)
        if (block_n == pool->free_list_index)
        {
            //pool->free_list = (Block *)pool->blocks[block_n].head.next;
            pool->free_list_index = pool->blocks[block_n].head.next;
            pool->blocks[block_n].head.prev = UINT16_MAX;
        }

        // remove the block from the free block list - cause it's not free no more
        uint16_t next_index = pool->blocks[block_n].head.next;
        uint16_t prev_index = pool->blocks[block_n].head.prev;

        if (next_index != UINT16_MAX)
        {
            pool->blocks[next_index].head.prev = prev_index;
        }
        if (prev_index != UINT16_MAX)
        {
            pool->blocks[prev_index].head.next = next_index;
        }

        pool->blocks[block_n].head.used = 1;
        block_n += pool->blocks[block_n].head.size;
    }
}

static size_t coalesce(Pool *pool)
{
    size_t count = 0;
    size_t i = 0;
    while ((i < pool->num_blocks) && (count < pool->num_blocks))
    {
        if (block_is_free(&pool->blocks[i]))
        {
            count++;
            join_adjacent(pool, i);
        }
        i += pool->blocks[i].head.size;
    }
    return count;
}

static void *alloc_from_free_list(Pool *pool, size_t sz)
{
    // calculate the number of blocks to allocate
    size_t N = sz + sizeof(AllocatedHead);
    size_t n_blocks = N / CHUNK_SIZE + ((N % CHUNK_SIZE != 0) * 1);

    // iterate over the free blocks
    //Block *n = pool->free_list;
    uint16_t n = pool->free_list_index;
    size_t count = 0;
    while ((n != UINT16_MAX) && (count < pool->num_blocks))
    {
        Block* block_n = &pool->blocks[n];
        count++;
        // if this block has sufficient space
        if (block_n->head.size >= n_blocks)
        {
            // if the block is too big, split it
            if (block_n->head.size > n_blocks)
            {
                // create second block at the end of this chunk of memory
                split_block(pool, block_n, n_blocks);
            }

            // if the free head is being allocated
            if (n == pool->free_list_index)
            {
                // move the free head to the next free chunk
                //pool->free_list = (Block *)n->head.next;
                pool->free_list_index = block_n->head.next;
                // link the next chunk to the prior chunk
                if (pool->free_list_index != UINT16_MAX)
                {
                    //pool->free_list->head.prev = n->head.prev;
                    pool->blocks[pool->free_list_index].head.prev = block_n->head.prev;
                }
            }
            else
            {
                // remove the block from the free block list - cause it's not free no more
                uint16_t next_index = block_n->head.next;
                uint16_t prev_index = block_n->head.prev;

                if (next_index != UINT16_MAX) {
                    pool->blocks[next_index].head.prev = block_n->head.prev;
                }
                if (prev_index != UINT16_MAX) {
                    pool->blocks[prev_index].head.next = block_n->head.next;
                }

            }
            block_n->head.used = 1; // block is now used
            // return a pointer to the start of the allocated chunk
            void* ptr = (void *)&block_n->data[sizeof(AllocatedHead)];
            return ptr;
        }
        //n = (Block *)block_n->head.next;
        n = block_n->head.next;
    }

    return NULL;
}

static void *pool_alloc(CtsAllocator *self, size_t size)
{
    Pool *pool = (Pool *)self;
    void *r = NULL;
    // first see if there is a suitable block
    r = alloc_from_free_list(pool, size);
    if (r == NULL)
    {
        // we're out of memory, so try joining together all the adjacent free blocks to see if they release a region large enough
        coalesce(pool);
        // after merging free blocks, there might be a large enough block free
        r = alloc_from_free_list(pool, size);
    }

    return r;
}

static void *pool_realloc(CtsAllocator *self, void *ptr, size_t size)
{
    Pool *pool = (Pool *)self;
    if (ptr == NULL)
    {
        return pool_alloc(self, size);
    }

    uint8_t *bptr = (uint8_t *)ptr;
    bptr -= sizeof(AllocatedHead);
    Block *block = (Block *)bptr;
    size_t old_chunks = block->head.size;

    // calculate the number of chunks to allocate
    size_t N = size + sizeof(AllocatedHead);
    size_t n_chunks = N / CHUNK_SIZE + ((N % CHUNK_SIZE != 0) * 1);

    // the allocation needs to be shrunk down
    if (old_chunks > n_chunks)
    {
        split_block(pool, block, n_chunks);
        return ptr;
    }
    // the allocation needs to grow larger
    else if (old_chunks < n_chunks)
    {
        // first attempt to join on any adjacent free blocks
        size_t blocknumber = (block - pool->blocks);
        join_adjacent(pool, blocknumber);

        if (block->head.size > n_chunks) // if the block size is now too big
        {
            split_block(pool, block, n_chunks); // split it
            return ptr;                   // return original pointer because nothing moved
        }
        else if (block->head.size == n_chunks) // if the block size is perfect
        {
            return ptr;
        }
        else // worst case scenario
        {
            // free the memory
            pool_free(self, ptr);

            // attempt allocation
            void *n = pool_alloc(self, size);
            if (n == NULL)
            {
                // un-free the block
                uint16_t blocknumber = (block - pool->blocks);
                if (blocknumber == pool->free_list_index)
                {
                    pool->free_list_index = block->head.next;
                    if (pool->free_list_index != UINT16_MAX)
                    {
                        //pool->free_list->head.prev = block->head.prev;
                        pool->blocks[pool->free_list_index].head.prev = block->head.prev;
                    }
                }
                else
                {
                    uint16_t next_index = block->head.next;
                    uint16_t prev_index = block->head.prev;

                    if (next_index != UINT16_MAX)
                    {
                        pool->blocks[next_index].head.prev = prev_index;
                    }
                    if (prev_index != UINT16_MAX)
                    {
                        pool->blocks[prev_index].head.next = next_index;
                    }
                }
                block->head.used = 1;
                return NULL;
            }
            // copy contents of the old chunks to the new location
            memmove(n, ptr, old_chunks * CHUNK_SIZE);
            return n;
        }
    }
    // the allocation is fine
    else
    {
        return ptr;
    }
}

static void pool_free(CtsAllocator *self, void *ptr)
{
    Pool *pool = (Pool *)self;
    if (ptr != NULL)
    {
        // freeing a block adds it to the head
        uint8_t *bptr = (uint8_t *)ptr;
        bptr -= sizeof(AllocatedHead);
        Block *block = (Block *)bptr;
        uint32_t block_index = block - pool->blocks;

        if (pool->free_list_index != UINT16_MAX)
        {
            pool->blocks[pool->free_list_index].head.prev = block_index;
        }

        block->head.next = pool->free_list_index;
        block->head.prev = UINT16_MAX;
        block->head.used = 0; 

        pool->free_list_index = block_index;
    }
}

CtsAllocator *cts_allocator_from_pool(void *pool_mem, size_t pool_size)
{
    // check if there's enough room for at least a Pool and a Block
    if (pool_size < sizeof(Pool) + sizeof(Block))
    {
        return NULL; // pool is too small
    }

    // clear the memory region
    memset(pool_mem, 0, pool_size);

    // Create the Pool at the start of the provided memory region
    Pool *pool = (Pool *)pool_mem;

    // The first Block starts immediately after the Pool
    uint8_t* ptr_bpool = (uint8_t *)pool_mem;
    ptr_bpool += sizeof(Pool);
    Block *block = (Block *)(ptr_bpool);
    

    // calculate the number of blocks that fit in the remaining space
    size_t num_blocks = (pool_size - sizeof(Pool)) / sizeof(Block);

    // initialize the first block
    block->head.size = num_blocks;
    block->head.next = UINT16_MAX;
    block->head.prev = UINT16_MAX;
    block->head.used = 0;

    pool->free_list_index = 0;
    pool->allocator.alloc = pool_alloc;
    pool->allocator.realloc = pool_realloc;
    pool->allocator.free = pool_free;
    pool->num_blocks = num_blocks;
    pool->blocks = block;

    return &pool->allocator;
}
