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

/**
 * @brief Создаёт новое адресное пространство: корневая таблица из пула
 * + identity-мегапейдж ядра БЕЗ бита U (пользователь ядро не видит).
 * Возвращает указатель на корень или NULL, если пул исчерпан.
 */
uint64_t* mmu_create_aspace(void);

/**
 * @brief Маппит одну страницу 4 КиБ: va -> pa с флагами R|W|X|U|...
 * Каталоги L2/L3 выделяются из пула по требованию.
 * Возвращает true при успехе, false при нехватке таблиц или конфликте маппинга.
 */
bool mmu_map_page(uint64_t* root, uintptr_t va, uintptr_t pa, uint64_t flags);

/** @brief Собирает значение satp для корневой таблицы (MODE_Sv39 | PPN). */
uint64_t mmu_satp_of(uint64_t* root);

/** @brief Переключает контекст памяти: запись satp + полный сброс TLB. */
void mmu_switch(uint64_t satp);

#endif /* MMU_H */
