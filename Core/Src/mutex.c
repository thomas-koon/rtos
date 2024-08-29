#include "mutex.h"
#include "kernel.h"
#include "list.h"
#include "main.h"
#include <stdio.h>
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

    char buffer[100];

    tcb_t * curr_task = get_current_task();

    if(mutex->task == NULL)
    {

        snprintf(buffer, 100, "Task %d took the mutex.\r\n", curr_task->id);
        UART_Print(buffer);

        mutex->task = curr_task;
        exit_critical();
        return;

    }

    snprintf(buffer, 100, "Task %d is waiting on the mutex held by Task %d.\r\n", curr_task->id, mutex->task->id);
    UART_Print(buffer);

    // priority inheritance
    // task in critical section gets highest priority of waiting tasks
    if(mutex->task->priority < curr_task->priority)
    {
        mutex->task->priority = curr_task->priority;
        snprintf(buffer, 100, "Mutex holder Task %d priority raised to: %d\r\n", mutex->task->id, curr_task->priority);
        UART_Print(buffer);
    }

    mutex_queue_task(mutex, curr_task);
    curr_task->state = TASK_BLOCKED;

    exit_critical();
    pend_yield();
}

void mutex_unlock(mutex_t * mutex)
{
    enter_critical();

    char buffer[100];

    // restore priority from inheritance
    snprintf(buffer, 100, "Restoring Task %d priority to %d.\r\n", mutex->task->id, mutex->task->og_priority);
    UART_Print(buffer);
    mutex->task->priority = mutex->task->og_priority;

    // no tasks waiting
    if(mutex->waiting_head == NULL)
    {
        snprintf(buffer, 100, "Task %d released the mutex.\r\n", get_current_task()->id);
        UART_Print(buffer);

        mutex->task = NULL;
        exit_critical();
        return;
    }

    mutex->task = mutex->waiting_head->task;

    set_task_ready(mutex->task);

    task_ll_remove(mutex->waiting_head->task, &(mutex->waiting_head));

    snprintf(buffer, 100, "Task %d unblocked.\r\n", mutex->task->id);
    UART_Print(buffer);

    exit_critical();

    pend_yield();
}

static void mutex_queue_task(mutex_t* mutex, tcb_t *task) 
{
    task_ll_insert(task, &(mutex->waiting_head));
}