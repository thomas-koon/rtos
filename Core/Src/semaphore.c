#include "semaphore.h"
#include "scheduler.h"
#include "main.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

static void sem_queue_task(sem_t* sem, sem_ll_node_t* new_node);

void sem_init(sem_t * sem, int init_count)
{
    sem->count = init_count;
    sem->max_count = init_count;
    sem->waiting_head = NULL;
    sem->waiting_tail = NULL;
}

void sem_post(sem_t * sem) 
{

    enter_critical();

    tcb_t * curr_task = get_current_task();
    
    if(sem->waiting_head == NULL && sem->count < sem->max_count) 
    {
        sem->count++; // if no tasks waiting, resource available
        exit_critical();
        return;
    }
    else
    {
        // dequeue the highest priority task from the head of the linked list
        sem_ll_node_t* head = sem->waiting_head;
        tcb_t* task = head->task;
        
        sem->waiting_head = sem->waiting_head->next;
        if (sem->waiting_head == NULL) 
        {
            sem->waiting_tail = NULL; // Queue becomes empty
        }

        free(head); 

        task->state = TASK_READY;
        insert_task_in_ready_queue(task);

        exit_critical();
        pend_yield();
    }

}

void sem_wait(sem_t * sem)
{

    enter_critical();

    tcb_t * curr_task = get_current_task();

    // if the resource is available, take it
    if (sem->count > 0) 
    {
        sem->count--; 
        exit_critical();
        return;
    }
    
    // resource not available: wait on the semaphore queue
    sem_ll_node_t * new_node = (sem_ll_node_t *) malloc(sizeof(sem_ll_node_t));
    
    if(new_node == NULL)
    {
        exit_critical();
        return;
    }

    new_node->task = curr_task;
    new_node->next = NULL;

    sem_queue_task(sem, new_node);

    curr_task->state = TASK_BLOCKED;

    exit_critical();

    pend_yield();

}

static void sem_queue_task(sem_t* sem, sem_ll_node_t* new_node) {
    sem_ll_node_t* prev = NULL;
    sem_ll_node_t* current = sem->waiting_head;

    while (current != NULL && current->task->priority >= new_node->task->priority) 
    {
        prev = current;
        current = current->next;
    }

    if (prev == NULL) 
    {
        // new_node is the new head
        sem->waiting_head = new_node;
    } else {
        prev->next = new_node;
    }

    new_node->next = current;

    if (current == NULL) 
    {
        // new_node is the new tail
        sem->waiting_tail = new_node;
    }
}