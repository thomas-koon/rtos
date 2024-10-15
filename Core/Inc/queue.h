#ifndef QUEUE_H
#define QUEUE_H

typedef struct msg_t
{
    void* msg;
    struct msg_t* next;
} msg_t; 

typedef struct msg_queue_t
{
    msg_t* msg_queue;
    int id;
    
} msg_queue_t;

#endif