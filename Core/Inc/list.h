#ifndef LIST_H
#define LIST_H

#include "task.h"

typedef struct task_ll_node_t
{
    tcb_t * task;
    struct task_ll_node_t * next;
} task_ll_node_t;

void task_ll_insert(tcb_t *task, task_ll_node_t **head);
void task_ll_remove(tcb_t *task, task_ll_node_t **head);

#endif