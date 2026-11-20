#ifndef MAIN_H
#define MAIN_H

#include "./include/types.h"
#include "./include/mm.h"
#include "./include/console.h"
#include "./include/sched.h"

// Регистры управления питанием QEMU (SiFive Test Finisher)
#define SYSCON_TEST_ADDR 0x100000
#define FINISHER_PASS 0x5555

// Константы битов прерываний RISC-V
#define MSTATUS_MIE (1ULL << 3) // Глобальное разрешение прерываний в Machine-mode
#define MIE_MTIE (1ULL << 7)    // Разрешение прерываний от машинного таймера

#define clear_csr(reg, bit) ({ asm volatile("csrc " #reg ", %0" ::"r"(bit)); })
#define write_csr(reg, val) ({ asm volatile("csrw " #reg ", %0" ::"r"(val)); })
#define set_csr(reg, bit) ({ asm volatile("csrs " #reg ", %0" ::"r"(bit)); })

#define BUFFER_SIZE 64

/// Частота таймера: 10 МГц для QEMU virt, 25 МГц для реального железа
#define TIMER_FREQ 10000000ULL

// // NOLINTBEGIN(bugprone-reserved-identifier)
// extern char _text_start[];
// extern char _text_end[];
// extern char _ram_end[];
// // NOLINTEND(bugprone-reserved-identifier)

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

// Регистры аппаратного таймера CLINT на плате QEMU virt (Hart 0)
#define CLINT_BASE 0x02000000UL
#define CLINT_MTIMECMP0 ((volatile uint64_t*)(CLINT_BASE + 0x4000))
#define CLINT_MTIME ((volatile uint64_t*)(CLINT_BASE + 0xBFF8))

// Квант времени планировщика: 100 мс при частоте таймера 10 МГц (1 000 000 тиков)
#define SCHED_QUANTUM_TICKS (TIMER_FREQ / 10)

// Код прерывания машинного таймера (старший бит 63 = 1, код = 7)
#define MCAUSE_TIMER_INTERRUPT ((1ULL << 63) | 7ULL)

// Макрос чтения CSR
#define read_csr(reg)                                                                                                  \
    ({                                                                                                                 \
        uint64_t __tmp;                                                                                                \
        asm volatile("csrr %0, " #reg : "=r"(__tmp));                                                                  \
        __tmp;                                                                                                         \
    })

// Коды исключений (mcause при старшем бите = 0)
#define CAUSE_INSTRUCTION_ACCESS_FAULT 1ULL
#define CAUSE_ILLEGAL_INSTRUCTION 2ULL
#define CAUSE_LOAD_ACCESS_FAULT 5ULL
#define CAUSE_STORE_ACCESS_FAULT 7ULL
#define CAUSE_ECALL_FROM_UMODE 8ULL
#define CAUSE_ECALL_FROM_MMODE 11ULL

// Флаги конфигурации PMP (биты: R=0, W=1, X=2, A[4:3]=01 для TOR)
#define PMP_R (1ULL << 0)
#define PMP_W (1ULL << 1)
#define PMP_X (1ULL << 2)
#define PMP_TOR (1ULL << 3)

void enable_interrupts(void);

// int kstrcmp(const char* s1, const char* s2);
// int kstrlen(const char* s);

uint64_t get_ticks(void);
void print_uptime(void);
void execute_command(const char* cmd);

// trap_frame_t* trap_handler(trap_frame_t* frame);

// void run_scheduler_demo(void);
// void enter_user_mode(uintptr_t entry, uintptr_t user_sp);
// void exit_user_mode_return(void);
// void run_user_mode_demo(void);

void run_pmp_violation_demo(void);


#endif