#include "../include/uart.h"
#include "../include/types.h"
#include <stdarg.h>

// #define UART0_THR ((volatile unsigned char*)(UART0_BASE + 0x00))
// #define UART0_LSR ((volatile unsigned char*)(UART0_BASE + 0x05))

/// Регистр данных UART (запись — transmit, чтение — receive)
#define UART0_THR (*(volatile uint8_t*)(UART0_BASE + 0x00))
/// Регистр статуса линии (бит 0 — Data Ready, бит 5 — THRE)
#define UART0_LSR (*(volatile uint8_t*)(UART0_BASE + 0x05))
#define LSR_THRE 0x20
#define LSR_DR 0x01

static inline void uart_write_reg(uint8_t reg, uint8_t val) {
    *(volatile uint8_t*)(UART0_BASE + reg) = val;
}

void uart_init(uint32_t baudrate, uart_databits_t data_bits, uart_parity_t parity, uart_stopbits_t stop_bits) {
    // 1. Отключаем прерывания UART
    uart_write_reg(UART_IER, 0x00);

    // 2. Открываем доступ к делителю частоты (DLAB = 1)
    uart_write_reg(UART_LCR, LCR_DLAB);

    // 3. Рассчитываем и записываем делитель под переданный baudrate
    uint16_t divisor = (uint16_t)(UART_CLOCK_HZ / (16 * baudrate));
    uart_write_reg(UART_DLL, (uint8_t)(divisor & 0xFF));
    uart_write_reg(UART_DLM, (uint8_t)((divisor >> 8) & 0xFF));

    // 4. Собираем конфигурацию кадра из переданных аргументов и сбрасываем DLAB в 0
    // Примечание: 1 стартовый бит (Start bit) генерируется контроллером аппаратно всегда.
    uint8_t lcr_config = (uint8_t)data_bits | (uint8_t)parity | (uint8_t)stop_bits;
    uart_write_reg(UART_LCR, lcr_config);

    // 5. Включаем и сбрасываем буферы FIFO
    uart_write_reg(UART_FCR, FCR_FIFO_EN | FCR_RX_RESET | FCR_TX_RESET);

    // 6. Активируем сигналы управления модемом (DTR, RTS)
    uart_write_reg(UART_MCR, 0x03);
}

/**
 * @brief Читает один символ из UART методом активного опроса (busy-wait).
 * @return Принятый символ (char).
 *
 * Крутится в цикле, пока бит Data Ready (LSR[0]) не станет равен 1,
 * затем возвращает символ из регистра данных.
 */
char uart_getc(void) {
    // Ждем, пока бит 0 (Data Ready) станет равен 1
    while ((UART0_LSR & LSR_DR) == 0) // БЕЗ звёздочки
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
    while ((UART0_LSR & LSR_THRE) == 0) // БЕЗ звёздочки
        ;
    UART0_THR = c; // присваивание, не разыменование
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
