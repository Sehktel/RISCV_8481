#include "poweroff.h"

/**
 * @brief Выполняет чистое выключение эмулятора QEMU.
 *
 * Записывает "магическое" значение @c FINISHER_PASS (0x5555) в устройство
 * sifive_test по адресу @c SYSCON_TEST_ADDR — QEMU завершает работу с кодом 0.
 *
 * @note На реальном железе этот адрес ничего не отключит: предусмотрена
 *       страховка — бесконечный цикл с инструкцией wfi (wait for interrupt).
 */
void poweroff(void) {
    volatile unsigned int* test_finisher = (volatile unsigned int*)SYSCON_TEST_ADDR;
    *test_finisher = FINISHER_PASS;

    // Предохранитель, если QEMU не отреагировал
    while (1) {
        __asm__ volatile("wfi");
    }
}
