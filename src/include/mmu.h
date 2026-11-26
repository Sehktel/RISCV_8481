#ifndef MMU_H
#define MMU_H

#include "types.h"

/**
 * @brief Инициализирует таблицы страниц Sv39 (этап 1: identity-мегапейдж).
 * Создаёт L1[2] = 0x80000000..0xBFFFFFFF (1 ГиБ, R|W|X|U|A|D).
 * Не включает MMU — включение отдельно в mmu_enable().
 */
void mmu_init(void);

/**
 * @brief Включает трансляцию: satp = MODE_Sv39 | корень таблиц.
 * Действует только в S/U-mode: M-mode при MPRV=0 транслируется мимо MMU.
 * Вызывать один раз из kmain до запуска U-mode задач.
 */
void mmu_enable(void);

#endif /* MMU_H */
