#include "pmp.h"
#include "csr.h"

#include "console.h"
#include "kernel_layout.h"

/* ── Биты поля pmpcfg ────────────────────────────────────────────────── */
#define PMP_CFG_R (1u << 0)
#define PMP_CFG_W (1u << 1)
#define PMP_CFG_X (1u << 2)
#define PMP_CFG_A_TOR (1u << 3) /* A = 01: TOR */
#define PMP_CFG_L (1u << 7)     /* НЕ ставим: ограничиваем только U-mode */

/**
 * @brief Настраивает аппаратную защиту памяти PMP (режим TOR) для процесса U-mode.
 *
 * Разрешает U-mode:
 *   - R+X для кода и констант [kernel_text_start, kernel_text_end)
 *   - R+W (без X, аппаратный NX) для собственного стека [ustack_base, ustack_top)
 * Запрещает прямой доступ к памяти ядра (.data, .bss, куча) и MMIO (UART0, CLINT).
 */
void pmp_configure_user(uintptr_t ustack_base, uintptr_t ustack_top) {
    // entry0: [0, kernel_text_start)           — запрет (UART 0x10000000, CLINT)
    // entry1: [kernel_text_start, text_end)    — R+X (код и константы)
    // entry2: [text_end, pgtables_start)       — запрет (.data, .bss, стек ядра)
    // entry3: [pgtables_start, pgtables_end)   — R (ОБХОД ТАБЛИЦ Sv39: MMU читает
    //                                             PTE с правами исходного доступа,
    //                                             здесь — U-mode fetch, обязателен)
    // entry4: [pgtables_end, ustack_base)      — запрет (куча: чужие kstack)
    // entry5: [ustack_base, ustack_top)        — R+W, NX (стек задачи)
    // entry6: [ustack_top, ram_end)            — запрет

    write_csr(pmpaddr0, (uintptr_t)kernel_text_start >> 2);
    write_csr(pmpaddr1, (uintptr_t)kernel_text_end >> 2);
    write_csr(pmpaddr2, (uintptr_t)kernel_pgtables_start >> 2);
    write_csr(pmpaddr3, (uintptr_t)kernel_pgtables_end >> 2);
    write_csr(pmpaddr4, ustack_base >> 2);
    write_csr(pmpaddr5, ustack_top >> 2);
    write_csr(pmpaddr6, ((uintptr_t)kernel_ram_end - 1) >> 2);

    uint64_t cfg = (PMP_TOR) |                         // entry0: низ закрыт
                   ((PMP_TOR | PMP_R | PMP_X) << 8) |  // entry1: код
                   ((PMP_TOR) << 16) |                 // entry2: запрет ядра
                   ((PMP_TOR | PMP_R) << 24) |         // entry3: таблицы, только чтение
                   ((PMP_TOR) << 32) |                 // entry4: запрет до стека
                   ((PMP_TOR | PMP_R | PMP_W) << 40) | // entry5: стек задачи
                   ((PMP_TOR) << 48);                  // entry6: верх закрыт
    write_csr(pmpcfg0, cfg);

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

/* ── Доступ к PMP CSR по индексу ───────────────────────────────────────
 * Имена регистров (pmpaddr0..7, pmpcfg0..3) вшиваются в инструкцию на
 * этапе компиляции, поэтому динамический индекс раскрывается через switch.
 * Виртуальные macropfor-макросы write_csr/read_csr здесь не подходят:
 * они строят имя регистра токеном и требуют константный номер. */

static inline void pmpaddr_csr_write(int idx, uintptr_t val) {
    switch (idx) {
    case 0:
        write_csr(pmpaddr0, val);
        break;
    case 1:
        write_csr(pmpaddr1, val);
        break;
    case 2:
        write_csr(pmpaddr2, val);
        break;
    case 3:
        write_csr(pmpaddr3, val);
        break;
    case 4:
        write_csr(pmpaddr4, val);
        break;
    case 5:
        write_csr(pmpaddr5, val);
        break;
    case 6:
        write_csr(pmpaddr6, val);
        break;
    case 7:
        write_csr(pmpaddr7, val);
        break;
    default:
        break; /* индекс за пределами конфигурации — игнорируем */
    }
}
static inline void pmpcfg_csr_write(int idx, uint8_t cfg) {
    const int reg = idx / 8; /* pmpcfg0..3: по 8 записей на CSR */
    const int shift = (idx % 8) * 8;
    const uint64_t mask = ~(0xFFULL << shift);
    uint64_t cur = 0;

    switch (reg) {
    case 0:
        cur = read_csr(pmpcfg0);
        break;
    case 1:
        cur = read_csr(pmpcfg1);
        break;
    case 2:
        cur = read_csr(pmpcfg2);
        break;
    case 3:
        cur = read_csr(pmpcfg3);
        break;
    default:
        return; /* неизвестный банк — молча выходим, ничего не пишем */
    }
    cur = (cur & mask) | ((uint64_t)cfg << shift);

    switch (reg) {
    case 0:
        write_csr(pmpcfg0, cur);
        break;
    case 1:
        write_csr(pmpcfg1, cur);
        break;
    case 2:
        write_csr(pmpcfg2, cur);
        break;
    case 3:
        write_csr(pmpcfg3, cur);
        break;
    default:
        break;
    }
}

/**
 * @brief Строгая карта PMP для U-mode: код ядра R|X, таблицы Sv39 R,
 * куча R|W, всё остальное — запрет. M-mode не ограничен (нет бита L).
 *
 * TOR: диапазон записи N = [pmpaddr[N-1] << 2 .. pmpaddr[N] << 2).
 * Нижняя граница записи 0 — физический 0. Записи строятся цепочкой:
 * верх каждой становится низом следующей, поэтому менять карту можно
 * только полным перестроением — одиночная правка сдвигает соседей.
 */
void pmp_configure_user_strict(void) {
    struct {
        uintptr_t end;
        uint8_t cfg;
    } map[] = {
        {(uintptr_t)kernel_text_start, 0},                   /* 0: до кода — запрет (вкл. UART) */
        {(uintptr_t)kernel_text_end, PMP_CFG_R | PMP_CFG_X}, /* 1: код ядра */
        {(uintptr_t)kernel_pgtables_start, 0},               /* 2: данные ядра — запрет */
        {(uintptr_t)kernel_pgtables_end, PMP_CFG_R},         /* 3: таблицы — обход MMU */
        {(uintptr_t)kernel_heap_start, 0},                   /* 4: зазор — запрет */
        {(uintptr_t)kernel_heap_end, PMP_CFG_R | PMP_CFG_W}, /* 5: куча */
    };
    const int n_entries = (int)(sizeof(map) / sizeof(map[0]));

    for (int i = 0; i < n_entries; i++) {
        pmpaddr_csr_write(i, map[i].end >> 2);
        pmpcfg_csr_write(i, map[i].cfg | PMP_CFG_A_TOR);
    }
    for (int i = n_entries; i < 16; i++) {
        pmpaddr_csr_write(i, 0);
        pmpcfg_csr_write(i, 0);
    }
}
