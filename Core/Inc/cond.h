#ifndef COND_H
#define COND_H

#include "mutex.h"

typedef struct
{
    mutex_t * mutex;
    task_ll_node_t * waiting_head;
} cond_t;

void cond_init(cond_t** cond);
void cond_wait(cond_t* cond, mutex_t* mutex);
void cond_signal(cond_t* cond);
void cond_broadcast(cond_t* cond);
void cond_free(cond_t** cond);

#endif