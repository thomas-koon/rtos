#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "rtos.h"

typedef struct ready_node_t
{
    tcb_t * task;
    struct ready_node_t * next;
} ready_node_t;

void scheduler_init(void);
void add_task(tcb_t *task);
void insert_task_in_ready_queue(tcb_t *task);
void remove_task_from_ready_queue(tcb_t *task);
tcb_t * get_next_task(void);
tcb_t * get_current_task(void);
void scheduler(void);

#endif // SCHEDULER_H
