#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "rtos.h"

typedef struct 
{
    tcb_t * task;
    struct sem_ll_node_t * next;
} sem_ll_node_t;

typedef struct 
{
    int count;
    int deadline_floor;
    sem_ll_node_t * waiting_head;
    sem_ll_node_t * waiting_tail;
} sem_t;

void sem_init(sem_t * sem, int init_count, int deadline_floor);
void sem_post(sem_t * sem);
void sem_wait(sem_t * sem);

#endif // SEMAPHORE_H