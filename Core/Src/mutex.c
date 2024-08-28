#include "mutex.h"
#include "kernel.h"
#include "list.h"
#include <stdlib.h>
#include <stddef.h>

static void mutex_queue_task(mutex_t* mutex, tcb_t* task);

void mutex_init(mutex_t ** mutex)
{
    *mutex = (mutex_t *) malloc(sizeof(mutex_t));
    (*mutex)->task = NULL;
    (*mutex)->waiting_head = NULL;
}

void mutex_lock(mutex_t * mutex)
{

    enter_critical();

    tcb_t * curr_task = get_current_task();

    if(mutex->task == NULL)
    {
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
        mutex->task = NULL;
        exit_critical();
        return;
    }

    mutex->task = mutex->waiting_head->task;
    mutex->task->state = TASK_READY;
    task_ll_remove(mutex->waiting_head->task, &(mutex->waiting_head));

    exit_critical();
}

static void mutex_queue_task(mutex_t* mutex, tcb_t *task) {
    task_ll_insert(task, &(mutex->waiting_head));
}