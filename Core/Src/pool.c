#include "pool.h"
#include "kernel.h"
#include "mutex.h"
#include "main.h"
#include "logger.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include "stm32f4xx_ll_cortex.h"

#define POOL_RAM_REGION_SIZE 8192 // 8 KB

__attribute__((section(".pool"))) char pool_data[POOL_RAM_REGION_SIZE];

uintptr_t pool_start = (uintptr_t)pool_data;

void pool_init(pool_t** pool, int num_blocks, int block_size)
{
    enter_critical();

    *pool = (pool_t*) malloc(sizeof(pool_t));

    (*pool)->free_list = (block_t*)pool_start;

    block_t* block = (*pool)->free_list;

    for(int i = 0; i < num_blocks; i++)
    {
        block->guard = 0xDEADBEEF; 
        block->next = (block_t*) ((char*)block + block_size);
        debug_log(DEBUG_POOL_INIT_BLOCK, block);
        block = block->next;
    }
    block->next = NULL; // last block

    (*pool)->block_size = block_size;
    (*pool)->blocks_left = num_blocks;
    (*pool)->num_blocks = num_blocks;

    // Initialize the entire pool as read-only using the MPU
    block = (*pool)->free_list;
    for (int region_num = 0; region_num < (num_blocks / 8); region_num++) 
    {
        // Set entire region (0x00) as read-only
        LL_MPU_ConfigRegion(region_num, 0x00, (uint32_t)block, LL_MPU_REGION_SIZE_2KB | LL_MPU_REGION_PRIV_RO);
        block = (block_t*) ((char*)block + (8 * block_size)); // Move to the next region
    }

    exit_critical();
}

void* pool_alloc(pool_t* pool)
{
    enter_critical();
    
    if (pool->free_list == NULL || pool->blocks_left == 0) 
    {
        exit_critical();
        return NULL;
    }

    block_t* allocated_start = pool->free_list;

    pool->free_list = pool->free_list->next; //allocated_start->next;
    pool->blocks_left--;

    debug_log(DEBUG_POOL_ALLOC, allocated_start);

    // MPU: Set this subregion to be writeable. 0 = read-only, 1 = writeable
    set_block_RW(allocated_start, pool);

    exit_critical();
    return (void*)allocated_start;
}


void pool_free(pool_t* pool, void* block)
{
    enter_critical();

    if(pool->blocks_left >= pool->num_blocks)
    {
        exit_critical();
        return;
    }

    block_t* freed_block = block;
    freed_block->guard = 0xDEADBEEF;
    freed_block->next = pool->free_list;
    pool->free_list = freed_block;
    pool->blocks_left++;
    
    debug_log(DEBUG_POOL_FREE, freed_block);

    // MPU: Set this subregion to be read-only.
    set_block_RO(freed_block, pool);

    exit_critical();
}

void pool_cleanup(pool_t* pool)
{
    enter_critical();
    free(pool->free_list);
    free(pool);
    exit_critical();
}

void set_block_RW(void* addr, pool_t* pool)
{
    enter_critical();

    // MPU: Set this subregion to be writeable. 0 = read-only, 1 = writeable
    uint32_t region_start_addr = (uint32_t)addr & ~0x7FF; // Get the start of the region (2KB aligned)
    uint32_t region_num = (region_start_addr - pool_start) / (pool->block_size * 8); 
    // Switch to the current region to use its RASR and RBAR values
    WRITE_REG(MPU->RNR, region_num);

    uint32_t subregion_index = (((uint32_t)addr - region_start_addr) / pool->block_size); // Subregion for this block
    uint32_t curr_srd = (MPU->RASR >> MPU_RASR_SRD_Pos) & 0xFF; // Current SRD value
    uint32_t new_srd = curr_srd | (1 << subregion_index);

    LL_MPU_ConfigRegion(region_num, new_srd, region_start_addr, LL_MPU_REGION_SIZE_2KB | LL_MPU_REGION_PRIV_RO);

    exit_critical();
}

void set_block_RO(void* addr, pool_t* pool)
{
    enter_critical();

    // MPU: Set this subregion to be read-only.
    uint32_t region_start_addr = (uint32_t)addr & ~0x7FF; // Get the start of the region (2KB aligned)
    uint32_t region_num = (region_start_addr - pool_start) / pool->block_size;
    // Switch to the current region to use its RASR and RBAR values
    WRITE_REG(MPU->RNR, region_num);

    uint32_t subregion_index = (((uint32_t)addr - region_start_addr) / pool->block_size); // Subregion for this block
    uint32_t curr_srd = (MPU->RASR >> MPU_RASR_SRD_Pos) & 0xFF; // Current SRD value
    uint32_t new_srd = curr_srd & ~(1 << subregion_index);

    LL_MPU_ConfigRegion(region_num, new_srd, region_start_addr, LL_MPU_REGION_SIZE_2KB | LL_MPU_REGION_PRIV_RO);

    exit_critical();
}