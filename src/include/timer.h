#ifndef TIMER_H
#define TIMER_H

#include "types.h"

/// Частота CLINT mtime в QEMU virt — 10 МГц
#define TIMER_FREQ_HZ 10000000UL

/// Частота таймера: 10 МГц для QEMU virt, 25 МГц для реального железа
#define CLINT_BASE 0x02000000UL
#define CLINT_MTIMECMP (*(volatile uint64_t*)(CLINT_BASE + 0x4000))
#define CLINT_MTIME ((volatile uint64_t*)(CLINT_BASE + 0xBFF8))

/// Планировщик: квант в тиках (10 мс → 100 000 тиков при 100 мс — 1 000 000)
#define SCHED_QUANTUM_MS 10
#define SCHED_QUANTUM_TICKS ((uint64_t)TIMER_FREQ_HZ * SCHED_QUANTUM_MS / 1000)

void timer_set_next(uint64_t quantum);
void timer_enable(bool enable);
uint64_t get_ticks(void);

#endif /* TIMER_H */