#include "uart.h"
#include "types.h"
#include <stdarg.h>

static inline void uart_write_reg(uint8_t reg, uint8_t val) {
    *(volatile uint8_t*)(UART0_BASE + reg) = val;
}

uart_err_t uart_init(uint32_t baudrate, uart_databits_t data_bits, uart_parity_t parity, uart_stopbits_t stop_bits) {
    // 1. Защита от деления на ноль и невалидного baudrate
    // Максимальный baudrate ограничен делителем = 1 (UART_CLOCK_HZ / 16)
    if (baudrate == 0 || baudrate > (UART_CLOCK_HZ / 16)) {
        return UART_ERR_INVALID_BAUD;
    }

    // Проверка битовых масок enum'ов на случай передачи мусора
    if ((data_bits & ~0x03) || (parity & ~0x38) || (stop_bits & ~0x04)) {
        return UART_ERR_INVALID_PARAM;
    }

    // Округление до ближайшего целого делителя и защита от переполнения 16 бит
    uint32_t divisor = (UART_CLOCK_HZ + (8 * baudrate)) / (16 * baudrate);
    if (divisor == 0 || divisor > 0xFFFF) {
        return UART_ERR_INVALID_BAUD;
    }

    // 2. Отключаем прерывания UART на время реконфигурации
    uart_write_reg(UART_IER, 0x00);

    // 3. Открываем доступ к делителю частоты (DLAB = 1)
    uart_write_reg(UART_LCR, LCR_DLAB);

    // 4. Записываем 16-битный делитель
    uart_write_reg(UART_DLL, (uint8_t)(divisor & 0xFF));
    uart_write_reg(UART_DLM, (uint8_t)((divisor >> 8) & 0xFF));

    // 5. Устанавливаем формат кадра (1 стартовый бит формируется аппаратно) и сбрасываем DLAB
    uint8_t lcr_config = (uint8_t)data_bits | (uint8_t)parity | (uint8_t)stop_bits;
    uart_write_reg(UART_LCR, lcr_config);

    // 6. Включаем и сбрасываем аппаратные буферы FIFO
    uart_write_reg(UART_FCR, FCR_FIFO_EN | FCR_RX_RESET | FCR_TX_RESET);

    // 7. Активируем DTR и RTS.
    // Примечание: в NS16550A бит MCR_OUT2 (0x08) стробирует линию IRQ от UART к контроллеру прерываний (PLIC).
    // Для polling-режима он не обязателен, но при переходе на прерывания нужно добавить `| MCR_OUT2`.
    uart_write_reg(UART_MCR, MCR_DTR | MCR_RTS);

    return UART_OK;
}

// void uart_init(uint32_t baudrate, uart_databits_t data_bits, uart_parity_t parity, uart_stopbits_t stop_bits) {
//     // 1. Отключаем прерывания UART
//     uart_write_reg(UART_IER, 0x00);

//     // 2. Открываем доступ к делителю частоты (DLAB = 1)
//     uart_write_reg(UART_LCR, LCR_DLAB);

//     // 3. Рассчитываем и записываем делитель под переданный baudrate
//     uint16_t divisor = (uint16_t)(UART_CLOCK_HZ / (16 * baudrate));
//     uart_write_reg(UART_DLL, (uint8_t)(divisor & 0xFF));
//     uart_write_reg(UART_DLM, (uint8_t)((divisor >> 8) & 0xFF));

//     // 4. Собираем конфигурацию кадра из переданных аргументов и сбрасываем DLAB в 0
//     // Примечание: 1 стартовый бит (Start bit) генерируется контроллером аппаратно всегда.
//     uint8_t lcr_config = (uint8_t)data_bits | (uint8_t)parity | (uint8_t)stop_bits;
//     uart_write_reg(UART_LCR, lcr_config);

//     // 5. Включаем и сбрасываем буферы FIFO
//     uart_write_reg(UART_FCR, FCR_FIFO_EN | FCR_RX_RESET | FCR_TX_RESET);

//     // 6. Активируем сигналы управления модемом (DTR, RTS)
//     uart_write_reg(UART_MCR, 0x03);
// }

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
