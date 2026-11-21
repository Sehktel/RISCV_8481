#include "../include/uart.h"
#include "../include/types.h"
#include <stdarg.h>


// #define UART0_THR ((volatile unsigned char*)(UART0_BASE + 0x00))
// #define UART0_LSR ((volatile unsigned char*)(UART0_BASE + 0x05))


/// Регистр данных UART (запись — transmit, чтение — receive)
#define UART0_THR (*(volatile uint8_t*)(UART0_BASE + 0x00))
/// Регистр статуса линии (бит 0 — Data Ready, бит 5 — THRE)
#define UART0_LSR (*(volatile uint8_t*)(UART0_BASE + 0x05))
#define LSR_THRE  0x20
#define LSR_DR    0x01

/**
 * @brief Читает один символ из UART методом активного опроса (busy-wait).
 * @return Принятый символ (char).
 *
 * Крутится в цикле, пока бит Data Ready (LSR[0]) не станет равен 1,
 * затем возвращает символ из регистра данных.
 */
char uart_getc(void) {
    // Ждем, пока бит 0 (Data Ready) станет равен 1
    while ((UART0_LSR & LSR_DR) == 0)      // БЕЗ звёздочки
        ;
    // Возвращаем считанный символ из регистра данных ( «чтение из RBR (тот же адрес, что THR на запись))
    return (char)UART0_THR;
}

/**
 * @brief Выводит один символ в UART.
 * @param c Символ для передачи.
 *
 * Перед записью ждёт освобождения передатчика: бит THRE (LSR[5])
 * должен быть равен 1.
 */
void uart_putc(char c) {
    while ((UART0_LSR & LSR_THRE) == 0)   // БЕЗ звёздочки
        ;
    UART0_THR = c;                         // присваивание, не разыменование
}


/**
 * @brief Выводит NUL-терминированную строку в UART.
 * @param str Указатель на строку (не NULL).
 *
 * Символ '\n' автоматически заменяется парой CR+LF ('\r' + '\n'),
 * как требует терминал QEMU в режиме nographic.
 */
void print(const char* str) {
    while (*str) {
        if (*str == '\n') {
            uart_putc('\r'); // Для терминалов nographic перенос строки требует CR+LF
        }
        uart_putc(*str++);
    }
}

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
