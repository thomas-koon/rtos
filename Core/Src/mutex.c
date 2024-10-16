#include "mutex.h"
#include "pool.h"
#include "kernel.h"
#include "list.h"
#include "logger.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

static void mutex_queue_task(mutex_t* mutex, tcb_t* task);

void mutex_init(mutex_t ** mutex)
{
    *mutex = (mutex_t *) pool_alloc(pool);
    (*mutex)->task = NULL;
    (*mutex)->waiting_head = NULL;
}

void mutex_lock(mutex_t * mutex)
{

    enter_critical();
    
    tcb_t * curr_task = get_current_task();

    debug_log(DEBUG_MUTEX_LOCK_CALLED_BY, (uint32_t) curr_task->id);

    if(mutex->task == NULL)
    {
        debug_log(DEBUG_MUTEX_TAKEN_BY, (uint32_t) curr_task->id);
        mutex->task = curr_task;
        
        exit_critical();
        return;

    }

    // priority inheritance
    // task in critical section gets highest priority of waiting tasks
    if(mutex->task->priority < curr_task->priority)
    {
        mutex->task->priority = curr_task->priority;
    }

    debug_log(DEBUG_WAIT_FOR_MUTEX_HELD_BY, (uint32_t) mutex->task->id);
    
    mutex_queue_task(mutex, curr_task);
    curr_task->state = TASK_BLOCKED;

    exit_critical();
    pend_yield();
}

void mutex_unlock(mutex_t * mutex)
{
    enter_critical();

    // restore priority from inheritance
    mutex->task->priority = mutex->task->og_priority;

    // no tasks waiting
    if(mutex->waiting_head == NULL)
    {

        debug_log(DEBUG_MUTEX_RELEASED_BY, (uint32_t) mutex->task->id);

        mutex->task = NULL;
        exit_critical();
        return;
    }

    debug_log(DEBUG_MUTEX_RELEASED_BY, (uint32_t) mutex->task->id);

    mutex->task = mutex->waiting_head->task;

    debug_log(DEBUG_MUTEX_TAKEN_BY, (uint32_t) mutex->task->id);

    set_task_ready(mutex->task);

    task_ll_remove(mutex->waiting_head->task, &(mutex->waiting_head));

    exit_critical();

    pend_yield();
}

static void mutex_queue_task(mutex_t* mutex, tcb_t *task) 
{
    task_ll_insert(task, &(mutex->waiting_head));
}