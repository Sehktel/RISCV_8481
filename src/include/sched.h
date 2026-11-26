#ifndef SCHED_H
#define SCHED_H

#include "trap.h"
#include "types.h"
// #include "timer.h"

bool task_create(int slot, void (*entry)(void));

void run_scheduler_demo(void);

// void enter_user_mode(uintptr_t entry, uintptr_t user_sp);
// void exit_user_mode_return(void);

void run_user_mode_demo(void);
void run_pmp_violation_demo(void);

uint64_t sys_call(uint64_t sys_num, uint64_t arg0, uint64_t arg1);

void sched_set_shell_return_sp(uint64_t sp);
void sched_clear_current_task(void);

#endif /* SCHED_H */
