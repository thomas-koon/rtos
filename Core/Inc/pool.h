#ifndef POOL_H
#define POOL_H

#include <stdint.h>

typedef struct block_t
{
    uint32_t guard;
    struct block_t* next;
} block_t;

typedef struct 
{
    int num_blocks;
    int block_size;
    block_t* free_list;
    int blocks_left;
} pool_t;

void pool_init(pool_t** pool, int num_blocks, int block_size);
void* pool_alloc(pool_t* pool);
void pool_free(pool_t* pool, void* block);
void pool_cleanup(pool_t* pool);
void set_block_RW(void* addr, pool_t* pool);
void set_block_RO(void* addr, pool_t* pool);

#endif