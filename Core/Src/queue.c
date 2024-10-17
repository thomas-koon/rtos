#include "queue.h"
#include "pool.h"
#include "main.h"
#include "kernel.h"
#include "logger.h"

#include <stdint.h>
#include <stddef.h>

void msg_queue_init(msg_queue_t** queue, int size)
{
    *queue = (msg_queue_t*) pool_alloc(pool);
    (*queue)->msg_head = NULL;
    (*queue)->msg_tail = NULL;
    (*queue)->size = 0;
    (*queue)->capacity = size;
    (*queue)->waiting_head_post = NULL;
    (*queue)->waiting_head_pend = NULL;
}

void msg_post(msg_queue_t* queue, uint32_t msg_content)
{
    enter_critical();

    msg_t* msg = (msg_t*) pool_alloc(pool);
    msg->data = msg_content;
    msg->next = NULL;

    debug_log(DEBUG_MQ_POSTER_TASK_ID, get_current_task()->id);

    // queue is full
    if(queue->size >= queue->capacity)
    {
        // block this task
        task_ll_insert(get_current_task(), &(queue->waiting_head_post));
        get_current_task()->state = TASK_BLOCKED;

        pend_yield();

        debug_log(DEBUG_MQ_BLOCK_ON_POST_TASK_ID, get_current_task()->id);

        exit_critical();
        
        // yields here
        // once unblocked, add the message

        enter_critical(); 
    }

    if(queue->msg_tail == NULL)
    {
        queue->msg_tail = msg;
    }
    else
    {
        queue->msg_tail->next = msg;
        queue->msg_tail = queue->msg_tail->next;
    }

    debug_log(DEBUG_MQ_DATA_POSTED, msg->data);

    // if a task is blocked pending on this queue
    if(queue->waiting_head_pend != NULL)
    {
        set_task_ready(queue->waiting_head_pend->task);
        task_ll_remove(queue->waiting_head_pend->task, &(queue->waiting_head_pend));
        pend_yield();
    }
    else
    {
        queue->size++;
    }

    uart_print("\r\n");

    exit_critical();
    return;
}

uint32_t msg_pend(msg_queue_t* queue)
{
    enter_critical();
    uint32_t msg_data;

    // no messages in queue
    if(queue->size == 0)
    {
        // block this task
        task_ll_insert(get_current_task(), &(queue->waiting_head_pend));
        get_current_task()->state = TASK_BLOCKED;

        pend_yield();

        exit_critical();

        // yields here
        // once unblocked, get the message

        enter_critical();
    }

    msg_data = queue->msg_head->data;
    queue->msg_head = queue->msg_head->next;

    queue->size--;


    exit_critical();
}