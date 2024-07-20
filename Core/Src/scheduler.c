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
static uint32_t num_tasks;

void scheduler_init(tcb_t * first_task) 
{
    
    for (uint32_t i = 0; i < MAX_TASKS; i++) 
    {
    
        tasks[i] = NULL;
    
    }

    curr_task = first_task;
    num_tasks = 0;

}

void add_task(tcb_t *task) 
{

    if(num_tasks < MAX_TASKS) 
    {
        num_tasks++;
        tasks[num_tasks] = task;
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

tcb_t* get_next_task(void) 
{
    return ready_queue_head->task;  
}

tcb_t* get_current_task(void)
{
    return curr_task;
}

void scheduler(void) 
{
    __disable_irq();

    // TODO: Handle expired task

    tcb_t * next_task = get_next_task();

    // if task in ready queue
    if (next_task != NULL && next_task->state == TASK_READY)
    {

        // if ready task has earlier deadline than current task
        if (curr_task == NULL || next_task->deadline <= curr_task->deadline)
        {
            
            // queue the current task
            if (curr_task != NULL && curr_task->state == TASK_RUNNING) 
            {
                curr_task->state = TASK_READY;
                insert_task_in_ready_queue(curr_task);
            }

            remove_task_from_ready_queue(next_task);
            next_task->state = TASK_RUNNING;

            // trigger PendSV to handle the context switch
            SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;

            curr_task = next_task;

        }

    }

    __enable_irq();
}

__attribute((naked)) void PendSV_Handler(void)
{
    __asm volatile 
    (
        // save the context of the current task 
        "     mrs r0, psp                     \n" // get current PSP
        "     isb                             \n"
        "     ldr r1, =curr_task              \n" // load address of curr_task into r1
        "     ldr r2, [r1]                    \n" // load pointer to curr_task's TCB
        "     stmdb r0!, {r4-r11}             \n" // store r4 - r11 on process stack
        "     str r0, [r2]                    \n" // Set curr_task->stack_top to PSP

        "     bl get_next_task                \n"

        // restore the context of the next task
        "     ldr r2, [r0]                    \n" // load pointer to next_task's TCB
        "     ldmia r2!, {r4-r11}             \n" // restore r4-r11 from next_task's stack
        "     msr psp, r2                     \n" // update PSP with next_task->stack_top
        "     bx lr                           \n"
    );
}