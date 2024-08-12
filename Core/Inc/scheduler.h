#ifndef switch_task_H
#define switch_task_H

#include "task.h"

typedef struct ready_node_t
{
    tcb_t * task;
    struct ready_node_t * next;
} ready_node_t;

void switch_task_init(tcb_t * first_task);
void add_task(tcb_t *task);
void suspend_task(tcb_t *task);
void resume_task(tcb_t *task);
tcb_t * get_task_by_id(int i);
void insert_task_in_ready_queue(tcb_t *task);
void remove_task_from_ready_queue(tcb_t *task);
tcb_t * get_current_task(void);
void switch_task(void);

#endif // switch_task_H
