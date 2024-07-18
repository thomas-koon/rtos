#include "rtos.h"
#include "scheduler.h"

static void init_task_stack(tcb_t *task, task_func_t task_func, void *parameters, uint32_t *stack, uint32_t stack_size);

void rtos_init(void)
{
    scheduler_init();
    
}

rtos_create_task(tcb_t *task, task_func_t task_func, void *parameters, uint32_t deadline, uint32_t *stack, uint32_t stack_size)
{
    task = (struct tcb_t *) malloc(sizeof(tcb_t));
    task->stack_top = stack + stack_size - 1;
    task->stack_bottom = stack;
    task->task_func = task_func;
    task->deadline = deadline;
    task->og_deadline = deadline;
    task->task_state_t = TASK_READY;

    init_task_stack(task, task_func, parameters, stack, stack_size);
}

static void init_task_stack(tcb_t *task, task_func_t task_func, void *parameters, uint32_t *stack, uint32_t stack_size)
{

    memset(stack, 0, stack_size * sizeof(uint32_t));

    uint32_t *stack_ptr = stack + stack_size - 1;
    *(--stack_ptr) = (1U << 24);           // xPSR (set bit 24 (T-bit) for thumb)
    *(--stack_ptr) = (uint32_t)task_func;  // PC (task function address)
    *(--stack_ptr) = 0xFFFFFFFD;           // LR (EXC_RETURN)
    stack_ptr -= 5;                        // R12, R3, R2, R1
    *(--stack_ptr) = (uint32_t)parameters; // R0
    stack_ptr -= 8;                        // R11, R10, R9, R8, R7, R6, R5 and R4.

    task->stack_top = stack_ptr; 

}
