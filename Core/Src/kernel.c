#include "main.h"
#include "kernel.h"
#include "list.h"
#include "pool.h"
#include "logger.h"
#include "stm32f4xx.h"  
#include "stm32f4xx_it.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "cmsis_gcc.h"


#define MAX_TASKS 10
#define MASK_PRIORITY        (0x06<<4)

static task_ll_node_t *ready_queue_head;
static tcb_t *tasks[MAX_TASKS];
static tcb_t *curr_task; 
static uint32_t num_tasks = 0;
static volatile uint32_t nested_critical = 0;

static void remove_task_from_ready_queue(tcb_t * task);

void scheduler_init(tcb_t * first_task) 
{
    remove_task_from_ready_queue(first_task);
    curr_task = first_task;
    curr_task->state = TASK_RUNNING;
    set_block_RW(curr_task->stack_bottom, pool);

    // From the ARM docs:
    // "In an RTOS environment, 
    // PendSV is usually configured to 
    // have the lowest interrupt priority level."

    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    NVIC_SetPriority(SysTick_IRQn, 0xFF);
    NVIC_SetPriority(PendSV_IRQn, 0xFF);

    SysTick->LOAD = (16000000 / 1) - 1;

    debug_log(DEBUG_KERNEL_STARTING, 0);

    __enable_irq();

    // Use inline assembly to branch to the task function
    asm volatile 
    (
        "svc 0"
    );
}

void add_task(tcb_t *task) 
{

    if(num_tasks < MAX_TASKS) 
    {   
        tasks[num_tasks] = task;
        num_tasks++;
        set_task_ready(task);
    }

}

void pend_yield()
{
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void suspend_task(tcb_t *task)
{
    __disable_irq();
    task->state = TASK_SUSPENDED;
    if(task != curr_task)
    {
        remove_task_from_ready_queue(task);
    }
    __enable_irq();
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void resume_task(tcb_t *task) {
    __disable_irq();
    if(task->state == TASK_SUSPENDED)
    {
        task->state = TASK_READY;
        set_task_ready(task);
    }
    __enable_irq();
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

tcb_t * get_task_by_id(int id)
{
    for(int i = 0; i < num_tasks; i++)
    {
        if(tasks[i]->id == id)
        {
            return tasks[i];
        }
    }
    return NULL;
}

void set_task_ready(tcb_t *task) 
{
    task->state = TASK_READY;
    task_ll_insert(task, &(ready_queue_head));
}

static void remove_task_from_ready_queue(tcb_t *task) 
{
    task_ll_remove(task, &(ready_queue_head));
}

tcb_t* get_current_task(void)
{
    return curr_task;
}

void mask_irq(void)
{

    // mask all numerically higher interrupts
    uint32_t tmp = MASK_PRIORITY;

    __asm volatile (
        " msr basepri, %0  \n\t"
        :
        : "r" (tmp)
    );
}

int last_t_bit = 0;

void unmask_irq(void)
{
    uint32_t tmp = 0;
    __set_BASEPRI(0);
    // __asm volatile (
    //     " msr basepri, %0  \n\t"
    //     :
    //     : "r" (tmp)
    // );
    last_t_bit = __get_xPSR() & (1 << 24);
}

int x = 0;

void enter_critical(void)
{
    mask_irq();
    ++nested_critical;

}

void exit_critical(void)
{
    x++;
    if(--nested_critical == 0)
        unmask_irq();
}


void switch_task(void) 
{

    mask_irq();
    if (curr_task == NULL) 
    {
        unmask_irq();
        return;
    }

    if (ready_queue_head == NULL) 
    {
        unmask_irq();
        return; // No other task to schedule
    }

    tcb_t *next_ready_task = ready_queue_head->task;

    if (next_ready_task != NULL && next_ready_task->state == TASK_READY) 
    {
        // If ready task has a higher or equal priority than current task, or current task suspended or blocked
        if (curr_task == NULL || next_ready_task->priority >= curr_task->priority || curr_task->state == TASK_SUSPENDED || curr_task->state == TASK_BLOCKED) 
        {
            remove_task_from_ready_queue(next_ready_task);

            // Queue the current task
            if (curr_task != NULL && curr_task->state == TASK_RUNNING) 
            {
                curr_task->state = TASK_READY;
                set_task_ready(curr_task);
            }

            next_ready_task->state = TASK_RUNNING;

            // Set curr_task stack to read only
            set_block_RO(curr_task->stack_bottom, pool);

            curr_task = next_ready_task;
            debug_log(DEBUG_TASK_SWITCHED_TO, (uint32_t) curr_task->id);
            
            // set (new) curr task stack writeable
            set_block_RW(curr_task->stack_bottom, pool);
        } 
    } 
    unmask_irq();
}

__attribute((naked)) void SVC_Handler(void)
{
    __asm volatile
    (
        "    ldr r3, =curr_task       \n" // Get the location of the current TCB
        "    ldr r1, [r3]                \n" // Load the TCB
        "    ldr r0, [r1]                \n" // Load the stack pointer from TCB
        "    ldmia r0!, {r4-r11, r14}   \n" // Restore the core registers
        "    msr psp, r0                 \n" // Update the PSP
        "    isb                         \n" // Ensure the update is complete
        "    bx r14                      \n" // Branch to the task function
        "    .align 4                    \n"
    );
}

__attribute((naked)) void PendSV_Handler(void)
{
    __asm volatile 
    (
        // Save the context of the current task
        "     mrs r0, psp                     \n" // get current PSP
        "     isb                             \n"
        "     ldr r3, =curr_task              \n" // load address of the pointer curr_task into r1
        "     ldr r2, [r3]                    \n" // load TCB pointed to by curr_task, into r2
        "     stmdb r0!, {r4-r11, r14}         \n" // store r4 - r11 on process stack
        "     str r0, [r2]                    \n" // Save new stack top

        "     stmdb sp!, {r0, r3}             \n" 
        "     dsb                             \n"
        "     isb                             \n"

        "     bl switch_task                    \n"
        "     ldmia sp!, {r0, r3}             \n"

        "     ldr r1, [r3]                    \n"
        "     ldr r0, [r1]                    \n" // get the (new) curr_task's stack_top

        "     ldmia r0!, {r4-r11, r14}        \n" // restore r4-r11 from stack

        "     msr psp, r0                     \n" // update PSP with stack_top
        "     isb                             \n"

        "     bx r14                          \n" // return to the next task
        
        "     .align 4                        \n"
    );
}
