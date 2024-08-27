#ifndef MUTEX_H
#define MUTEX_H

#include "task.h"
#include "list.h"

typedef struct
{
    tcb_t * task;
    task_ll_node_t * waiting_head;
} mutex_t;

void mutex_init(mutex_t * mutex);
void mutex_lock(mutex_t * mutex);
void mutex_unlock(mutex_t * mutex);

#endif // MUTEX_H