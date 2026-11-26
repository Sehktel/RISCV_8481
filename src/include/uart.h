#ifndef UART_H
#define UART_H

// Адрес UART0 на плате QEMU virt
#define UART0_BASE 0x10000000UL

#define UART0_BASE 0x10000000UL
#define UART_CLOCK_HZ 3686400UL

// Смещения регистров NS16550A
#define UART_DLL 0x00 // Divisor Latch Low (DLAB=1)
#define UART_IER 0x01 // Interrupt Enable (DLAB=0)
#define UART_DLM 0x01 // Divisor Latch High (DLAB=1)
#define UART_FCR 0x02 // FIFO Control
#define UART_LCR 0x03 // Line Control
#define UART_MCR 0x04 // Modem Control

#define LCR_DLAB 0x80
#define FCR_FIFO_EN 0x01
#define FCR_RX_RESET 0x02
#define FCR_TX_RESET 0x04

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

void uart_init();

char uart_getc(void);
void uart_putc(char c);
void print(const char* str);

#endif /* UART_H */