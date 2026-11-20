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
    while ((*UART0_LSR & (1 << 0)) == 0)
        ;
    // Возвращаем считанный символ из регистра данных (он же UART0_THR на чтение)
    return (char)(*UART0_THR);
}

/**
 * @brief Выводит один символ в UART.
 * @param c Символ для передачи.
 *
 * Перед записью ждёт освобождения передатчика: бит THRE (LSR[5])
 * должен быть равен 1.
 */
void uart_putc(char c) {
    while ((*UART0_LSR & (1 << 5)) == 0)
        ; // Ожидание свободного буфера передачи
    *UART0_THR = c;
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
