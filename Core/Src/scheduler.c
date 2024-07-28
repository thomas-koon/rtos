#include "scheduler.h"
#include "stm32f4xx.h"  
#include "stm32f4xx_it.h"
#include <stddef.h>
#include <stdlib.h>

#define MAX_TASKS 10

static ready_node_t *ready_queue_head;
static ready_node_t *ready_queue_tail;
static tcb_t *tasks[MAX_TASKS];
static tcb_t *curr_task;
static tcb_t *next_task;
static uint32_t num_tasks = 0;

void scheduler_init(tcb_t * first_task) 
{
    remove_task_from_ready_queue(first_task);
    curr_task = first_task;
    curr_task->state = TASK_RUNNING;

    update_next_task();

    __set_PSP((uint32_t)first_task->stack_top);
    __set_CONTROL(0x02);
    __ISB();

    // Use inline assembly to branch to the task function
    asm volatile 
    (
        "ldr r0, [%1]       \n" // Load the value of first_task->parameters into R0 (first parameter)
        "ldr r1, [%0]       \n" // Load the value of first_task->task_func into R1 (function pointer)
        "mov lr, #0xFFFFFFFD\n" // Set the Link Register (LR) to the EXC_RETURN value for thread mode, using PSP
        "bx r1              \n" // Branch to the address in R1 (task function)
        : 
        : "r"(&first_task->task_func), "r"(&first_task->parameters) // Input operands
        : "r0", "r1" // Clobbered registers
    );
}

void add_task(tcb_t *task) 
{

    if(num_tasks < MAX_TASKS) 
    {   
        tasks[num_tasks] = task;
        num_tasks++;
        insert_task_in_ready_queue(task);
    }

}

void insert_task_in_ready_queue(tcb_t *task) 
{
    ready_node_t *prev = NULL;
    ready_node_t *current = ready_queue_head;

    while (current != NULL && current->task->deadline <= task->deadline) 
    {
        prev = current;
        current = current->next;
    }

    ready_node_t * task_node = (struct ready_node_t*) malloc(sizeof(ready_node_t));
    task_node->task = task;

    if (prev == NULL) 
    {

        task_node->next = ready_queue_head;
        ready_queue_head = task_node;
    } 
    else 
    {
        task_node->next = prev->next;
        prev->next = task_node;
    }

    if (task_node->next == NULL) 
    {
        ready_queue_tail = task_node;
    }

    update_next_task();
}

void remove_task_from_ready_queue(tcb_t *task) 
{
    ready_node_t *prev = NULL;
    ready_node_t *current = ready_queue_head;

    while (current != NULL && current->task != task) 
    {
        prev = current;
        current = current->next;
    }

    if (current != NULL) 
    {
        if (prev == NULL) 
        {
            ready_queue_head = current->next;
        } 
        else 
        {
            prev->next = current->next;
        }

        if (current->next == NULL) 
        {
            ready_queue_tail = prev;
        }
        current->next = NULL;
    }

}

/**
 * @details Called when adding a task, removing a task, or scheduling.
 *          Use this before removing a task in scheduler.
 *          Used for telling the PendSV handler the next task to run. 
 */
void update_next_task(void) 
{
    if(ready_queue_head->task->deadline <= curr_task->deadline)
    {
        next_task = ready_queue_head->task;  
    }
    else
    {
        next_task = curr_task;    
    }
   
}

tcb_t* get_current_task(void)
{
    return curr_task;
}

void scheduler(void) 
{

    __disable_irq();

    // TODO: Handle expired task

    tcb_t * next_ready_task = ready_queue_head->task;

    // if task in ready queue
    if (next_ready_task != NULL && next_ready_task->state == TASK_READY)
    {

        // if ready task has earlier deadline than current task
        if (curr_task == NULL || next_ready_task->deadline <= curr_task->deadline)
        {
            
            // queue the current task
            if (curr_task != NULL && curr_task->state == TASK_RUNNING) 
            {
                curr_task->state = TASK_READY;
                insert_task_in_ready_queue(curr_task);
            }

            update_next_task();
            remove_task_from_ready_queue(next_ready_task);
            next_ready_task->state = TASK_RUNNING;

            // trigger PendSV to handle the context switch
            SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;

            // curr_task will be updated in PendSV_Handler
        }

    }

    __enable_irq();
}

__attribute((naked)) void PendSV_Handler(void)
{
    __asm volatile 
    (
        // Save the context of the current task
        "     mrs r0, psp                     \n" // get current PSP
        "     isb                             \n"
        "     ldr r1, =curr_task              \n" // load address of the pointer curr_task into r1
        "     ldr r2, [r1]                    \n" // load TCB pointed to by curr_task, into r2
        "     stmdb r0!, {r4-r11}             \n" // store r4 - r11 on process stack
        "     str r0, [r2]                    \n" // Save new stack top

        // Update curr_task to next_task
        "     ldr r0, =next_task              \n" // load address of next_task into r0
        "     ldr r2, [r0]                    \n" // load pointer to next_task's TCB
        "     str r2, [r1]                    \n" // update curr_task to point to next_task

        // Restore the context of the next task
        "     ldr r0, [r2]                    \n" // get the (new) curr_task's stack_top
        "     ldmia r0!, {r4-r11}             \n" // restore r4-r11 from stack
        "     msr psp, r0                     \n" // update PSP with stack_top
        "     bx lr                           \n" // return to the next task
    );
}
