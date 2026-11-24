#include "../include/pmp.h"
#include "../include/csr.h"

/**
 * @brief Настраивает аппаратную защиту памяти PMP (режим TOR) для процесса U-mode.
 *
 * Разрешает U-mode:
 *   - R+X для кода и констант [_text_start, _text_end)
 *   - R+W (без X, аппаратный NX) для собственного стека [ustack_base, ustack_top)
 * Запрещает прямой доступ к памяти ядра (.data, .bss, куча) и MMIO (UART0, CLINT).
 */
void pmp_configure_user(uintptr_t ustack_base, uintptr_t ustack_top) {
    // entry0: [0, _text_start)        — запрет (MMIO: UART, CLINT)
    // entry1: [_text_start, _rodata_end) — R+X (код + константы, читаемые U-mode)
    // entry2: [_rodata_end, ustack_base) — запрет (.data, .bss, куча)
    // entry3: [ustack_base, ustack_top)  — R+W, NX (стек задачи)
    // entry4: [ustack_top, конец RAM)    — запрет (куча выше, стеки других задач)

    write_csr(pmpaddr0, (uintptr_t)_text_start >> 2);
    write_csr(pmpaddr1, (uintptr_t)_text_end >> 2);
    write_csr(pmpaddr2, ustack_base >> 2);
    write_csr(pmpaddr3, ustack_top >> 2);
    write_csr(pmpaddr4, ((uintptr_t)_ram_end - 1) >> 2);

    uint64_t cfg = (PMP_TOR) |                         // entry0: низ закрыт
                   ((PMP_TOR | PMP_R | PMP_X) << 8) |  // entry1: код
                   ((PMP_TOR) << 16) |                 // entry2: запрет ядра
                   ((PMP_TOR | PMP_R | PMP_W) << 24) | // entry3: стек задачи
                   ((PMP_TOR) << 32);                  // entry4: верх закрыт
    write_csr(pmpcfg0, cfg);

    // Сброс трансляций после изменения правил защиты
    asm volatile("sfence.vma zero, zero" ::: "memory");
}

/* Дефолтный профиль U-задач: один TOR-регион на всё пространство с R+W+X.
 * M-mode не ограничен (нет бита L). Строгие профили (pmptest, per-task в
 * schedule_next) переопределяют его на время своего входа в U-mode. */
void pmp_set_default_user_profile(void) {
    /* Регион 0: TOR, база 0, верх — всё адресное пространство. R+W+X, без L:
     * ограничения видит только U-mode. Ровно та конфигурация, что стояла
     * при загрузке ядра до появления демо. */
    write_csr(pmpaddr0, (~0ULL) >> 10);
    write_csr(pmpcfg0, PMP_R | PMP_W | PMP_X | PMP_TOR);
}
