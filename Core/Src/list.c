#include "list.h"
#include "main.h"
#include "pool.h"
#include <stddef.h>
#include <stdlib.h>

void task_ll_insert(tcb_t *task, task_ll_node_t **head)
{
    task_ll_node_t *prev = NULL;
    task_ll_node_t *curr = *head;

    // Traverse the list to find the correct position based on task priority
    while (curr != NULL && curr->task->priority >= task->priority)
    {
        prev = curr;
        curr = curr->next;
    }

    task_ll_node_t *task_node = (task_ll_node_t*) pool_alloc(pool);
    if (task_node == NULL) 
    {
        return;
    }

    task_node->task = task;
    task_node->next = curr; // curr has less priority than task_node

    // Insert at the head of the queue
    if (prev == NULL) 
    {
        *head = task_node;
    } 
    else 
    {
        prev->next = task_node;
    }

}

void task_ll_remove(tcb_t *task, task_ll_node_t **head)
{

    task_ll_node_t *prev = NULL;
    task_ll_node_t *curr = *head;

    // Traverse the list to find the task to remove
    while (curr != NULL && curr->task != task) 
    {
        prev = curr;
        curr = curr->next;
    }

    if (curr != NULL) 
    {
        // Task found, now remove it from the list
        if (prev == NULL) 
        {
            // Task is at the head of the queue
            *head = curr->next;
        } 
        else 
        {
            // Task is in the middle or end
            prev->next = curr->next;
        }

        // Clear the next pointer of the removed node
        curr->next = NULL;

        pool_free(pool, curr);
    }

}