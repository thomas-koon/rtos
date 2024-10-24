#ifndef LOGGER_H
#define LOGGER_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#define LOG_BUFFER_SIZE 2048

typedef struct {
    char buffer[LOG_BUFFER_SIZE];
    volatile uint16_t head;  // data is written here
    volatile uint16_t tail;  // data is read here (for TX)
} log_buffer_t;

#define DEBUG_SPACER                    0xFFFF

// pool
#define DEBUG_POOL_INIT_BLOCK           0x0001
#define DEBUG_POOL_ALLOC                0X0002
#define DEBUG_POOL_FREE                 0x0003
#define DEBUG_POOL_CURR_FREE_LIST       0x0004

// mutex
#define DEBUG_MUTEX_LOCK_CALLED_BY      0x0005
#define DEBUG_MUTEX_TAKEN_BY            0x0006
#define DEBUG_WAIT_FOR_MUTEX_HELD_BY    0x0007
#define DEBUG_MUTEX_RELEASED_BY         0x0008

// kernel
#define DEBUG_TASK_SWITCHED_TO          0x0009
#define DEBUG_KERNEL_STARTING           0x000A
#define DEBUG_CURR_TASK                 0x000B

// message queue
#define DEBUG_MQ_POSTER_TASK_ID         0x000C
#define DEBUG_MQ_DATA_POSTED            0x000D
#define DEBUG_MQ_BLOCK_ON_POST_TASK_ID  0x000E
#define DEBUG_MQ_PENDER_TASK_ID         0x000F
#define DEBUG_MQ_DATA_RECEIVED          0x0010
#define DEBUG_MQ_BLOCK_ON_PEND_TASK_ID  0x0011

// semaphore debug
#define DEBUG_SEM_POST_TASK             0x0012
#define DEBUG_SEM_WAIT_TASK             0x0013
#define DEBUG_SEM_NEW_COUNT             0x0014
#define DEBUG_SEM_TASK_BLOCKED          0x0015
#define DEBUG_SEM_TASK_AWAKEN           0x0016

// condition variable
#define DEBUG_CV_WAIT_BLOCK             0x0017
#define DEBUG_CV_WAIT_WAKE              0x0018
#define DEBUG_CV_SIGNAL                 0x0019

// general purpose / misc
#define DEBUG_MISC_A                    0x0020
#define DEBUG_MISC_B                    0x0021
#define DEBUG_MISC_C                    0x0022
#define DEBUG_MISC_D                    0x0023
#define DEBUG_MISC_E                    0x0024
#define DEBUG_MISC_F                    0x0025


void debug_log(uint32_t id, uint32_t param);
void uart_dma_send();

#endif