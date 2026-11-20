#ifndef UART_H
#define UART_H

// Адрес UART0 на плате QEMU virt
#define UART0_BASE 0x10000000UL

char uart_getc(void);
void uart_putc(char c);
void print(const char* str);
void poweroff(void);

#endif /* UART_H */
