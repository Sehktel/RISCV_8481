#ifndef UART_H
#define UART_H

// Адрес UART0 на плате QEMU virt
#define UART0_BASE 0x10000000UL

// Регистры управления питанием QEMU (SiFive Test Finisher)
#define SYSCON_TEST_ADDR 0x100000
#define FINISHER_PASS 0x5555


char uart_getc(void);
void uart_putc(char c);
void print(const char* str);
void poweroff(void);

#endif /* UART_H */
