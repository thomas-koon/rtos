#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "rtos.h"

typedef struct 
{
    tcb_t * task;
    struct ready_node_t * next;
} ready_node_t;

void scheduler_init(void);
void scheduler_add_task(tcb_t *task);
tcb_t * scheduler_get_next_task(void);

#endif // SCHEDULER_H
