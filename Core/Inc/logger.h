#ifndef LOGGER_H
#define LOGGER_H

#include "stm32f4xx_hal.h"

// pool debug statements
#define DEBUG_POOL_INIT_BLOCK           0x0001
#define DEBUG_POOL_ALLOC                0X0002
#define DEBUG_POOL_FREE                 0x0003
#define DEBUG_POOL_CURR_FREE_LIST       0x0004

// mutex debug statements
#define DEBUG_MUTEX_LOCK_CALLED_BY      0x0005
#define DEBUG_MUTEX_TAKEN_BY            0x0006
#define DEBUG_WAIT_FOR_MUTEX_HELD_BY    0x0007
#define DEBUG_MUTEX_RELEASED_BY         0x0008

// kernel debug statements
#define DEBUG_TASK_SWITCHED_TO           0x0009
#define DEBUG_KERNEL_STARTING            0x000A
#define DEBUG_CURR_TASK                  0x000B

#define DEBUG_GENERIC                    0xFFFF

void debug_log(uint32_t id, uint32_t param);
void uart_print(const char *str);

#endif