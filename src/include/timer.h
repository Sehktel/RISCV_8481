#ifndef TIMER_H
#define TIMER_H

#include "types.h"

/// Частота таймера: 10 МГц для QEMU virt, 25 МГц для реального железа
#define TIMER_FREQ 10000000ULL
#define SCHED_QUANTUM_MS 100
#define SCHED_QUANTUM_TICKS (TIMER_FREQ_HZ * SCHED_QUANTUM_MS / 1000)

// Регистры аппаратного таймера CLINT на плате QEMU virt (Hart 0)
#define CLINT_BASE 0x02000000UL
#define CLINT_MTIMECMP (*(volatile uint64_t*)(CLINT_BASE + 0x4000))
#define CLINT_MTIME ((volatile uint64_t*)(CLINT_BASE + 0xBFF8))
#define TIMER_FREQ_HZ 10000000UL

void timer_set_next(uint64_t quantum);
void timer_enable(bool enable);
uint64_t get_ticks(void);

#endif /* TIMER_H */