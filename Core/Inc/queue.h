#ifndef QUEUE_H
#define QUEUE_H

#include "list.h"
#include <stdint.h>

typedef struct msg_t
{
    uint32_t data;
    struct msg_t* next;
} msg_t; 

typedef struct msg_queue_t
{
    msg_t* msg_head; // Pend from here 
    msg_t* msg_tail;      // Post to here
    int capacity;
    int size;
    task_ll_node_t* waiting_head_post;
    task_ll_node_t* waiting_head_pend;
} msg_queue_t;

void msg_queue_init(msg_queue_t** queue, int size);
void msg_post(msg_queue_t* queue, uint32_t msg);
uint32_t msg_pend(msg_queue_t* queue);
void msg_pend_no_timeout(msg_queue_t* queue);

#endif