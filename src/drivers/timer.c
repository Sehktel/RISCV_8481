#include "../include/timer.h"
#include "../include/csr.h"
#include "../include/sched.h"

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

uint64_t get_ticks(void) {
    return read_csr(time);
}
