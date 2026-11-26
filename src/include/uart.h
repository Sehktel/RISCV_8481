#ifndef UART_H
#define UART_H

#include "types.h"

// Адрес UART0 на плате QEMU virt
#define UART0_BASE 0x10000000UL
#define UART_CLOCK_HZ 3686400UL

/// Регистр данных UART (запись — transmit, чтение — receive)
#define UART0_THR (*(volatile uint8_t*)(UART0_BASE + 0x00))
/// Регистр статуса линии (бит 0 — Data Ready, бит 5 — THRE)
#define UART0_LSR (*(volatile uint8_t*)(UART0_BASE + 0x05))
#define LSR_THRE 0x20
#define LSR_DR 0x01

// Смещения регистров NS16550A
#define UART_DLL 0x00 // Divisor Latch Low (DLAB=1)
#define UART_IER 0x01 // Interrupt Enable (DLAB=0)
#define UART_DLM 0x01 // Divisor Latch High (DLAB=1)
#define UART_FCR 0x02 // FIFO Control
#define UART_LCR 0x03 // Line Control
#define UART_MCR 0x04 // Modem Control

// Биты регистров
#define LCR_DLAB 0x80
#define FCR_FIFO_EN 0x01
#define FCR_RX_RESET 0x02
#define FCR_TX_RESET 0x04

#define MCR_DTR 0x01
#define MCR_RTS 0x02
#define MCR_OUT2 0x08 // Разрешает прохождение прерываний UART на PLIC

// Типы параметров для удобного и безопасного вызова
typedef enum {
    UART_DATA_5BITS = 0x00,
    UART_DATA_6BITS = 0x01,
    UART_DATA_7BITS = 0x02,
    UART_DATA_8BITS = 0x03
} uart_databits_t;

typedef enum {
    UART_PARITY_NONE = 0x00,
    UART_PARITY_ODD = 0x08,
    UART_PARITY_EVEN = 0x18,
    UART_PARITY_MARK = 0x28,
    UART_PARITY_SPACE = 0x38
} uart_parity_t;

typedef enum {
    UART_STOP_1BIT = 0x00,
    UART_STOP_2BITS = 0x04 // Для 5 бит данных аппаратно формируется 1.5 стоп-бита
} uart_stopbits_t;

// Определение типов ДО функции uart_init
typedef enum { UART_OK = 0, UART_ERR_INVALID_BAUD = -1, UART_ERR_INVALID_PARAM = -2 } uart_err_t;

uart_err_t uart_init(uint32_t baudrate, uart_databits_t data_bits, uart_parity_t parity, uart_stopbits_t stop_bits);

char uart_getc(void);
void uart_putc(char c);
void print(const char* str);

#endif /* UART_H */