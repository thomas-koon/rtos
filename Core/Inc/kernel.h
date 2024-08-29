#ifndef KERNEL_H
#define KERNEL_H

#include "task.h"

void scheduler_init(tcb_t * first_task);
void add_task(tcb_t *task);
void pend_yield();

void suspend_task(tcb_t *task);
void resume_task(tcb_t *task);

tcb_t * get_task_by_id(int i);
tcb_t * get_current_task(void);

void set_task_ready(tcb_t * task);

void switch_task(void);

void mask_irq(void);
void unmask_irq(void);

void enter_critical(void);
void exit_critical(void);

#endif // KERNEL_H
