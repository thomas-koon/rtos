#include "semaphore.h"
#include "scheduler.h"
#include <stddef.h>

static void sem_queue_task(sem_t* sem, sem_ll_node_t* new_node);

void sem_init(sem_t * sem, int init_count, int deadline_floor)
{
    sem->count = init_count;
    sem->deadline_floor = deadline_floor;
    sem->waiting_head = NULL;
    sem->waiting_tail = NULL;
}

void sem_post(sem_t * sem) 
{

    __disable_irq();

    tcb_t * curr_task = rtos_get_current_task();
    curr_task->deadline = curr_task->og_deadline;
    
    if(sem->waiting_head == NULL) 
    {
        sem->count++; // if no tasks waiting, resource available
        __enable_irq();
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

        __enable_irq();
        scheduler();
    }

    

}

void sem_wait(sem_t * sem)
{

    __disable_irq();

    tcb_t * curr_task = rtos_get_current_task();

    uint32_t new_deadline = get_clock() + sem->deadline_floor;

    if (new_deadline < curr_task->deadline) 
    {
        curr_task->deadline = new_deadline;  
    }

    // if the resource is available
    if (sem->count > 0) 
    {
        sem->count--; 
        __enable_irq();
        return;
    }
    
    // resource not available: wait on the semaphore queue
    sem_ll_node_t * new_node = (sem_ll_node_t *) malloc(sizeof(sem_ll_node_t));
    
    if(new_node == NULL)
    {
        __enable_irq();
        return;
    }

    new_node->task = curr_task;
    new_node->next = NULL;

    sem_queue_task(sem, new_node);

    curr_task->state = TASK_BLOCKED;

    __enable_irq();

    scheduler();

}

static void sem_queue_task(sem_t* sem, sem_ll_node_t* new_node) {
    sem_ll_node_t* prev = NULL;
    sem_ll_node_t* current = sem->waiting_head;

    while (current != NULL && current->task->deadline < new_node->task->deadline) 
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