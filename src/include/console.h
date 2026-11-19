#ifndef CONSOLE_H
#define CONSOLE_H

#include "types.h"


// Адрес UART0 на плате QEMU virt
#define UART0_BASE 0x10000000UL

/// Регистр данных UART (запись — transmit, чтение — receive)
#define UART0_THR ((volatile unsigned char*)(UART0_BASE + 0x00))
/// Регистр статуса линии (бит 0 — Data Ready, бит 5 — THRE)
#define UART0_LSR ((volatile unsigned char*)(UART0_BASE + 0x05))


char uart_getc(void);
void uart_putc(char c);
void print(const char* str);
void print_number(uint64_t num, int base, int min_width);
void kprintf(const char* fmt, ...);

#endif