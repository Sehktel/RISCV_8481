#include "../include/timer.h"
#include "../include/csr.h"
#include "../include/sched.h"

// void timer_set_next(uint64_t ticks) {
//     uint64_t next = read_csr(time) + ticks;
//     *(volatile uint64_t*)CLINT_MTIMECMP0 = next;
// }
void timer_set_next(uint64_t ticks) {
    uint64_t next = read_csr(time) + ticks;
    CLINT_MTIMECMP = next;
}

void timer_enable(bool enable) {
    if (enable) {
        /* Вооружаем ПЕРВЫЙ тик: после сброса mtimecmp = 0, и MTIP
         * уже pending — без этого прерывание сработает мгновенно,
         * не дав enter_user_mode дойти до mret (и CPU окажется
         * в U-mode на M-mode инструкции enter_user_mode). */
        timer_set_next(SCHED_QUANTUM_TICKS);
        set_csr(mie, (1ULL << 7)); /* MTIE — machine timer interrupt enable */
    } else {
        clear_csr(mie, (1ULL << 7));
    }
}

// void timer_enable(bool enable) {
//     if (enable)
//         set_csr(mie, (1ULL << 7)); /* MTIE — machine timer interrupt enable */
//     else
//         clear_csr(mie, (1ULL << 7));
// }

uint64_t get_ticks(void) {
    return read_csr(time);
}

// #include "../include/timer.h"
// #include "../include/sched.h"
// #include "../include/csr.h"

// /**
//  * @brief Планирует следующее прерывание таймера CLINT через quantum тиков.
//  */
// void timer_set_next(uint64_t quantum) {
//     *CLINT_MTIMECMP0 = *CLINT_MTIME + quantum;
// }

// /**
//  * @brief Включает или отключает прерывания машинного таймера (MIE.MTIE).
//  */
// void timer_enable(bool enable) {
//     if (enable) {
//         timer_set_next(SCHED_QUANTUM_TICKS);
//         set_csr(mie, MIE_MTIE);
//     } else {
//         clear_csr(mie, MIE_MTIE);
//     }
// }

// /**
//  * @brief Читает 64-битный счётчик аппаратного времени (mtime).
//  * @return Текущее значение счётчика в тиках.
//  *
//  * В QEMU virt тики идут с частотой TIMER_FREQ (10 МГц).
//  * Инструкция rdtime атомарно читает CSR time, так что разрыв
//  * между старшим и младшим словом невозможен.
//  */
// uint64_t get_ticks(void) {
//     uint64_t ticks;
//     asm volatile("rdtime %0" : "=r"(ticks));
//     return ticks;
// }
