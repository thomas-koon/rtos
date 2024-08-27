#include "semaphore.h"
#include "kernel.h"
#include "main.h"
#include "list.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

static void sem_queue_task(sem_t* sem, tcb_t* task);

void sem_init(sem_t **sem, int init_count) 
{
    *sem = (sem_t *)malloc(sizeof(sem_t));
    if (*sem == NULL) 
    {
        return; 
    }
    (*sem)->count = init_count;
    (*sem)->max_count = init_count;
    (*sem)->waiting_head = NULL;
}


void sem_post(sem_t * sem) 
{

    enter_critical();

    char buffer[100];

    tcb_t * curr_task = get_current_task();
    
    if(sem->waiting_head == NULL && sem->count < sem->max_count) 
    {
        sem->count++; // if no tasks waiting, resource available
        snprintf(buffer, 100, "Task %d released semaphore. New count: %d\r\n", curr_task->id, sem->count);
        UART_Print(buffer);
        exit_critical();
        return;
    }
    else
    {
        // dequeue the highest priority task from the head of the linked list
        task_ll_node_t* head = sem->waiting_head;
        tcb_t* task = head->task;
        
        sem->waiting_head = sem->waiting_head->next;

        free(head); 

        task->state = TASK_READY;
        insert_task_in_ready_queue(task);

        snprintf(buffer, 100, "Task %d released semaphore. Task %d is now ready.\r\n", curr_task->id, task->id);
        UART_Print(buffer);

        exit_critical();
        pend_yield();
    }

}

void sem_wait(sem_t * sem)
{

    enter_critical();

    tcb_t * curr_task = get_current_task();
    char buffer[100];

    // if the resource is available, take it
    if (sem->count > 0) 
    {
        sem->count--; 

        snprintf(buffer, 100, "Task %d acquired semaphore. New count: %d\r\n", curr_task->id, sem->count);
        UART_Print(buffer);

        exit_critical();
        return;
    }
    else
    {
        sem_queue_task(sem, curr_task);

        curr_task->state = TASK_BLOCKED;

        snprintf(buffer, 100, "Task %d is blocked and added to semaphore queue.\r\n", curr_task->id);
        UART_Print(buffer);

        exit_critical();

        pend_yield();
        
    }

}

static void sem_queue_task(sem_t* sem, tcb_t *task) {
    task_ll_insert(task, &(sem->waiting_head));
}