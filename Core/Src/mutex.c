#include "mutex.h"
#include "list.h"
#include <stdlib.h>
#include <stddef.h>

void mutex_init(mutex_t * mutex)
{
    mutex = (mutex_t *) malloc(sizeof(mutex_t));
    mutex->task = NULL;
    mutex->waiting_head = NULL;
}