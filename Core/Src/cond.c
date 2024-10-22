#include "cond.h"
#include "kernel.h"
#include "pool.h"
#include "main.h"
#include "mutex.h"
#include "logger.h"

#include <stddef.h>

void cond_init(cond_t** cond)
{
    *cond = pool_alloc(pool);
    (*cond)->mutex = NULL;
    (*cond)->waiting_head = NULL;
}

void cond_wait(cond_t* cond, mutex_t* mutex)
{
    enter_critical();
    tcb_t* curr_task = get_current_task();

    // from OSTEP:
    // "wait always
    // (a) assumes the lock is held when you call it, 
    // (b) releases said lock when putting the caller to sleep, and
    // (c) re-acquires the lock just before returning"
    
    mutex_unlock(mutex);

    // block this task
    task_ll_insert(curr_task, &(cond->waiting_head));
    curr_task->state = TASK_BLOCKED;
    pend_yield();

    debug_log(DEBUG_CV_WAIT_BLOCK, curr_task->id);

    exit_critical();

    // yields here

    enter_critical();
    
    debug_log(DEBUG_CV_WAIT_WAKE, curr_task->id);

    mutex_lock(mutex);

    exit_critical();    
}

void cond_signal(cond_t* cond)
{
    enter_critical();
    tcb_t* curr_task = get_current_task();

    if(cond->waiting_head != NULL)
    {
        set_task_ready(cond->waiting_head->task);
        task_ll_remove(cond->waiting_head->task, &(cond->waiting_head));

        debug_log(DEBUG_CV_SIGNAL, curr_task->id);
    }
    
    exit_critical();
}