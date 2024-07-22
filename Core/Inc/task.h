#ifndef RTOS_H
#define RTOS_H

#include <stdint.h>

// Tasks in RTOS don't exit; return void
typedef void (*task_func_t)(void *);

typedef enum
{
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED, // delay, waiting on semaphore, etc
    TASK_SUSPENDED
} task_state_t;

typedef struct 
{
    uint32_t *stack_top;
    uint32_t *stack_bottom;
    task_func_t task_func;
    void *parameters;
    uint32_t deadline;
    uint32_t og_deadline;
    task_state_t state;
} tcb_t;

void create_task(tcb_t **task, task_func_t task_func, void *parameters, uint32_t deadline, uint32_t *stack, uint32_t stack_size);
void update_task_deadline(uint32_t deadline);

#endif // RTOS_H
