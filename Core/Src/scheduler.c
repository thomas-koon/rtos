#include "main.h"
#include "scheduler.h"
#include "stm32f4xx.h"  
#include "stm32f4xx_it.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_TASKS 10
#define MAX_SYSCALL_INTERRUPT_PRIORITY (5 << 4)

static ready_node_t *ready_queue_head;
static tcb_t *tasks[MAX_TASKS];
static tcb_t *curr_task; 
static uint32_t num_tasks = 0;

void switch_task_init(tcb_t * first_task) 
{
    remove_task_from_ready_queue(first_task);
    curr_task = first_task;
    curr_task->state = TASK_RUNNING;

    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    NVIC_SetPriority(SysTick_IRQn, 0x0F);
    NVIC_SetPriority(PendSV_IRQn, 0x0F);

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
        insert_task_in_ready_queue(task);
    }

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
        insert_task_in_ready_queue(task);
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

void insert_task_in_ready_queue(tcb_t *task) 
{
    ready_node_t *prev = NULL;
    ready_node_t *current = ready_queue_head;

    // Traverse the list to find the correct position based on task priority
    while (current != NULL && current->task->priority >= task->priority) 
    {
        prev = current;
        current = current->next;
    }

    // Allocate memory for the new node
    ready_node_t *task_node = (ready_node_t*) malloc(sizeof(ready_node_t));
    if (task_node == NULL) 
    {
        // Handle memory allocation failure
        return;
    }
    task_node->task = task;
    task_node->next = current; // The new node points to the current node

    if (prev == NULL) 
    {
        // Insert at the head of the queue
        ready_queue_head = task_node;
    } 
    else 
    {
        // Insert in between or at the end
        prev->next = task_node;
    }

}

void remove_task_from_ready_queue(tcb_t *task) 
{
    ready_node_t *prev = NULL;
    ready_node_t *current = ready_queue_head;

    // Traverse the list to find the task to remove
    while (current != NULL && current->task != task) 
    {
        prev = current;
        current = current->next;
    }

    if (current != NULL) 
    {
        // Task found, now remove it from the list
        if (prev == NULL) 
        {
            // Task is at the head of the queue
            ready_queue_head = current->next;
        } 
        else 
        {
            // Task is in the middle or end
            prev->next = current->next;
        }

        // Clear the next pointer of the removed node
        current->next = NULL;

        // Optionally free the node if it was dynamically allocated
        // free(current);
    }
}

tcb_t* get_current_task(void)
{
    return curr_task;
}

void switch_task(void) 
{

    if (curr_task == NULL) 
    {
        return;
    }

    // Print current task info
    char buffer[100];

    // Print ready queue info
    ready_node_t *node = ready_queue_head;
    while (node != NULL) 
    {
        node = node->next;
    }

    if (ready_queue_head == NULL) 
    {
        return; // No other task to schedule
    }

    tcb_t *next_ready_task = ready_queue_head->task;

    if (next_ready_task != NULL && next_ready_task->state == TASK_READY) 
    {
        // If ready task has a higher or equal priority than current task, or current task suspended
        if (curr_task == NULL || next_ready_task->priority >= curr_task->priority || curr_task->state == TASK_SUSPENDED) 
        {

            remove_task_from_ready_queue(next_ready_task);

            // Queue the current task
            if (curr_task != NULL && curr_task->state == TASK_RUNNING) 
            {
                curr_task->state = TASK_READY;
                insert_task_in_ready_queue(curr_task);
            }

            next_ready_task->state = TASK_RUNNING;

            curr_task = next_ready_task;
        } 
    } 
    
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
        "     mov r0, %0                      \n"
        "     msr basepri, r0                 \n"
        "     dsb                             \n"
        "     isb                             \n"
        "     bl switch_task                    \n"

        "     mov r0, #0                      \n"
        "     msr basepri, r0                 \n"
        "     ldmia sp!, {r0, r3}             \n"

        "     ldr r1, [r3]                    \n"
        "     ldr r0, [r1]                    \n" // get the (new) curr_task's stack_top

        "     ldmia r0!, {r4-r11, r14}        \n" // restore r4-r11 from stack

        "     msr psp, r0                     \n" // update PSP with stack_top
        "     isb                             \n"

        "     bx r14                          \n" // return to the next task
        
        "     .align 4                        \n"
        ::"i"(MAX_SYSCALL_INTERRUPT_PRIORITY)
    );
}
