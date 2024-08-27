#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "task.h"
#include "list.h"

typedef struct 
{
    int count;
    int max_count;
    task_ll_node_t * waiting_head;
} sem_t;

void sem_init(sem_t ** sem, int init_count);
void sem_post(sem_t * sem);
void sem_wait(sem_t * sem);

#endif // SEMAPHORE_H