#include "mmu.h"
#include "console.h"
#include "csr.h"

/* ── Биты PTE (Sv39) ─────────────────────────────────────────────────────── */
#define PTE_V (1ULL << 0) /* Валидная запись */
#define PTE_R (1ULL << 1) /* Чтение */
#define PTE_W (1ULL << 2) /* Запись */
#define PTE_X (1ULL << 3) /* Исполнение */
#define PTE_U (1ULL << 4) /* Доступно в U-mode */
#define PTE_G (1ULL << 5) /* Глобальная */
#define PTE_A (1ULL << 6) /* Accessed  — обязателен: без SADT железо не ставит само */
#define PTE_D (1ULL << 7) /* Dirty     — обязателен для записи, иначе Store Fault */

#define PTE_PPN_SHIFT 10

/* satp: MODE=8 (Sv39) в битах 63:60, PPN корневой таблицы в битах 43:0 */
#define SATP_MODE_SV39 (8ULL << 60)
#define SATP_PPN_SHIFT 12

/* ── Таблицы: статические в BSS, выравнивание — страница ──────────────────
 * Не из кучи: таблицы живут вечно, а куча отравлена (инвариант 0xA5) и
 * на этапе 2 ещё и станет видимым пользователю PMP-регионом — копаться в
 * метаданных трансляции в ней недопустимо. */
__attribute__((aligned(4096))) static uint64_t l1_root[512];
__attribute__((aligned(4096))) static uint64_t l2_user[512];

/* Физический адрес из значения линкер-символа: у нас identity, адрес == PA */
static uint64_t pte_megapage(uintptr_t pa, uint64_t flags) {
    /* Мегапейдж L1: PPN2 = PA[37:30], размещается в битах 28..53 PTE */
    return (((uint64_t)pa >> 30) << 28) | flags;
}

void mmu_init(void) {
    for (int i = 0; i < 512; i++) {
        l1_root[i] = 0;
        l2_user[i] = 0;
    }

    /* L1[2]: identity-мегапейдж на весь гигабайт 0x80000000.
     * U|R|W|X — временный компромисс этапа 1: пользователь пока видит ядро
     * по identity. Реальную границу держит PMP (он от MMU не зависит).
     * На этапе 2 бит U снимается — пользователь получит только свои VA. */
    l1_root[2] = pte_megapage(0x80000000UL, PTE_V | PTE_R | PTE_W | PTE_X | PTE_U | PTE_A | PTE_D);

    /* L1[256]: VA 0x4000000000 — каталог L2 под пользовательский регион этапа 2.
     * Пока каталог пуст: вся пользовательская память создаётся постранично. */
    l1_root[256] = ((((uint64_t)(uintptr_t)l2_user) >> 12) << PTE_PPN_SHIFT) | PTE_V;

    kprintf("[MMU] Sv39 инициализирован: L1@0x%x, L2@0x%x, identity 1GiB @0x80000000\n", (uintptr_t)l1_root,
            (uintptr_t)l2_user);
}

void mmu_enable(void) {
    uint64_t satp = SATP_MODE_SV39 | ((((uint64_t)(uintptr_t)l1_root) >> SATP_PPN_SHIFT));

    /* Flush TLB обязателен: без sfence.vma MMU не увидит свежие PTE */
    __asm__ volatile("sfence.vma zero, zero\n"
                     "csrw satp, %0\n"
                     "sfence.vma zero, zero\n" ::"r"(satp)
                     : "memory");

    kprintf("[MMU] Трансляция включена (satp=0x%x) — действует в S/U-mode\n", (uintptr_t)satp);
}
