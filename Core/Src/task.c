#include "task.h"
#include "main.h"
#include "scheduler.h"
#include <string.h>
#include <stdlib.h>

#define EXEC_RETURN	( 0xfffffffd )
#define START_ADDRESS_MASK ( 0xfffffffeUL )

static void task_exit_error(void);
static void init_task_stack(tcb_t *task, task_func_t task_func, void *parameters, uint32_t *stack, uint32_t stack_size);

void create_task(tcb_t **task, task_func_t task_func, void *parameters, uint32_t priority, uint32_t *stack, uint32_t stack_size, uint8_t id)
{
    *task = (tcb_t *) malloc(sizeof(tcb_t));
    
    uint32_t *stack_top = stack + stack_size - 1;
    stack_top = (uint32_t *)((uint32_t)stack_top & ~0x7); // Ensure 8-byte alignment

    (*task)->stack_top = stack_top;

    (*task)->stack_bottom = stack;
    (*task)->task_func = task_func;
    (*task)->priority = priority;
    (*task)->og_priority = priority;
    (*task)->state = TASK_READY;
    (*task)->id = id;

    init_task_stack(*task, task_func, parameters, stack, stack_size);

    add_task(*task);
}

static void init_task_stack(tcb_t *task, task_func_t task_func, void *parameters, uint32_t *stack, uint32_t stack_size)
{

    memset(stack, 0, stack_size * sizeof(uint32_t));

    uint32_t *stack_ptr = stack + stack_size - 1;
    //stack_ptr = (uint32_t *)((uint32_t)stack_ptr & ~0x7); // Ensure 8-byte alignment

    // Needs to be 8-byte aligned
    if ((uint32_t) stack_ptr & 0x04) 
    {
        stack_ptr--;
    }

    *stack_ptr = (1U << 24);           // xPSR (set bit 24 (T-bit) for thumb)
    stack_ptr--;
    *stack_ptr = ((uint32_t)task_func) & START_ADDRESS_MASK;  // PC (task function address)
    stack_ptr--;
    *stack_ptr = (uint32_t)task_exit_error;           // LR 
    stack_ptr -= 5;                        // R12, R3, R2, R1
    *stack_ptr = (uint32_t)parameters; // R0

    stack_ptr--;
    *stack_ptr = EXEC_RETURN;

    stack_ptr -= 8;                        // R11, R10, R9, R8, R7, R6, R5 and R4.

    task->stack_top = stack_ptr; 

}


static void task_exit_error(void)
{
	/*
    * Used to catch tasks that try to return from their function.
    */
	UART_Print("no\r\n");
    __disable_irq();
	for( ;; );
}