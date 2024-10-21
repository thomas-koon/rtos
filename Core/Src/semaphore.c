#include "semaphore.h"
#include "kernel.h"
#include "pool.h"
#include "main.h"
#include "list.h"
#include "logger.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

static void sem_queue_task(sem_t* sem, tcb_t* task);

void sem_init(sem_t **sem, int init_count) 
{
    *sem = (sem_t *) pool_alloc(pool);
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

    tcb_t * curr_task = get_current_task();
    
    debug_log(DEBUG_SPACER, 0);
    debug_log(DEBUG_SEM_POST_TASK, curr_task->id);

    if(sem->waiting_head == NULL && sem->count < sem->max_count) 
    {
        sem->count++; // if no tasks waiting, resource available
        
        debug_log(DEBUG_SEM_NEW_COUNT, sem->count);

        debug_log(DEBUG_SPACER, 0);

        exit_critical();
        return;
    }
    else
    {
        // dequeue the highest priority task from the head of the linked list
        task_ll_node_t* head = sem->waiting_head;
        tcb_t* task = head->task;
        
        sem->waiting_head = sem->waiting_head->next;

        pool_free(pool, head);

        set_task_ready(task);

        debug_log(DEBUG_SEM_TASK_AWAKEN, task->id);

        exit_critical();
        pend_yield();
    }

    debug_log(DEBUG_SPACER, 0);

}

void sem_wait(sem_t * sem)
{

    enter_critical();

    tcb_t * curr_task = get_current_task();
    debug_log(DEBUG_SPACER, 0);
    debug_log(DEBUG_SEM_WAIT_TASK, curr_task->id);

    // if the resource is available, take it
    if (sem->count > 0) 
    {
        sem->count--; 
        
        debug_log(DEBUG_SEM_NEW_COUNT, sem->count);

        debug_log(DEBUG_SPACER, 0);

        exit_critical();
        return;
    }
    else
    {
        sem_queue_task(sem, curr_task);

        curr_task->state = TASK_BLOCKED;

        debug_log(DEBUG_SEM_TASK_BLOCKED, curr_task->id);

        exit_critical();

        pend_yield();
        
    }

    debug_log(DEBUG_SPACER, 0);

}

static void sem_queue_task(sem_t* sem, tcb_t *task) {
    task_ll_insert(task, &(sem->waiting_head));
}