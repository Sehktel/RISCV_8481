#ifndef TIMER_H
#define TIMER_H

#include "../include/types.h"

// Регистры аппаратного таймера CLINT на плате QEMU virt (Hart 0)
#define CLINT_BASE 0x02000000UL
#define CLINT_MTIMECMP (*(volatile uint64_t*)(CLINT_BASE + 0x4000))
#define CLINT_MTIME ((volatile uint64_t*)(CLINT_BASE + 0xBFF8))
#define TIMER_FREQ_HZ 10000000UL

void timer_set_next(uint64_t quantum);
void timer_enable(bool enable);
uint64_t get_ticks(void);

#endif /* TIMER_H */