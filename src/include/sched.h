#ifndef SCHED_H
#define SCHED_H

#include "types.h"

// Состояния потока и дескриптор задачи (Task Control Block)
#define MAX_TASKS 4

/// Частота таймера: 10 МГц для QEMU virt, 25 МГц для реального железа
#define TIMER_FREQ 10000000ULL
#define SCHED_QUANTUM_MS 100
#define SCHED_QUANTUM_TICKS (TIMER_FREQ_HZ * SCHED_QUANTUM_MS / 1000)

// Константы битов прерываний и привилегий RISC-V
#define MSTATUS_MIE (1ULL << 3)       // Глобальное разрешение прерываний в Machine-mode
#define MSTATUS_MPIE (1ULL << 7)      // Предыдущее состояние MIE до входа в ловушку
#define MSTATUS_MPP_MASK (3ULL << 11) // Маска режима привилегий (00=U, 01=S, 11=M)
#define MIE_MTIE (1ULL << 7) // Разрешение прерываний от машинного таймера

// Коды исключений (mcause при старшем бите = 0)
#define CAUSE_ILLEGAL_INSTRUCTION 2ULL
#define CAUSE_ECALL_FROM_UMODE 8ULL
#define CAUSE_ECALL_FROM_MMODE 11ULL

// Таблица системных вызовов (ABI RISC-V: номер в a7, аргументы в a0..a2)
#define SYS_PRINT 1ULL
#define SYS_GET_TICKS 2ULL
#define SYS_EXIT 93ULL

// Код прерывания машинного таймера (старший бит 63 = 1, код = 7)
#define MCAUSE_TIMER_INTERRUPT ((1ULL << 63) | 7ULL)

// Коды исключений (mcause при старшем бите = 0)
#define CAUSE_INSTRUCTION_ACCESS_FAULT 1ULL
#define CAUSE_ILLEGAL_INSTRUCTION 2ULL
#define CAUSE_LOAD_ACCESS_FAULT 5ULL
#define CAUSE_STORE_ACCESS_FAULT 7ULL
#define CAUSE_ECALL_FROM_UMODE 8ULL
#define CAUSE_ECALL_FROM_MMODE 11ULL

// typedef enum { TASK_READY, TASK_RUNNING, TASK_DONE } task_state_t;

typedef enum { TASK_UNUSED = 0, TASK_READY = 1, TASK_RUNNING = 2, TASK_EXITED = 3 } task_state_t;

// Структура сохранённого контекста (должна строго соответствовать смещениям в trap_vector)
typedef struct {
    uint64_t ra;
    uint64_t gp;
    uint64_t tp;
    uint64_t t0;
    uint64_t t1;
    uint64_t t2;
    uint64_t s0;
    uint64_t s1;
    uint64_t a0;
    uint64_t a1;
    uint64_t a2;
    uint64_t a3;
    uint64_t a4;
    uint64_t a5;
    uint64_t a6;
    uint64_t a7;
    uint64_t s2;
    uint64_t s3;
    uint64_t s4;
    uint64_t s5;
    uint64_t s6;
    uint64_t s7;
    uint64_t s8;
    uint64_t s9;
    uint64_t s10;
    uint64_t s11;
    uint64_t t3;
    uint64_t t4;
    uint64_t t5;
    uint64_t t6;
    uint64_t sp;
    uint64_t padding;
} trap_frame_t;

_Static_assert(sizeof(trap_frame_t) == 256, "trap_frame_t must match 256-byte assembly frame in start.S");

typedef struct {
    int id;
    task_state_t state;
    uint64_t mepc;   // Сохранённый счётчик команд потока
    uint8_t* kstack; // Выделенный в куче стек ядра (на вершине лежит trap_frame_t)
    uint8_t* ustack; // Выделенный в куче стек пользователя
    trap_frame_t* frame; // Указатель на контекст 32 регистров в kstack
} task_t;

trap_frame_t* schedule_next(trap_frame_t* current_frame);
bool task_create(int slot, void (*entry)(void));

trap_frame_t* trap_handler(trap_frame_t* frame);

void run_scheduler_demo(void);
void enter_user_mode(uintptr_t entry, uintptr_t user_sp);
void exit_user_mode_return(void);
void run_user_mode_demo(void);
void run_pmp_violation_demo(void);

uint64_t sys_call(uint64_t sys_num, uint64_t arg0, uint64_t arg1);

void sched_set_shell_return_sp(uint64_t sp);
void sched_clear_current_task(void);

#endif /* SCHED_H */
