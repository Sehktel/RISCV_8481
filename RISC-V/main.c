#define UART0_BASE 0x10000000
#define UART0_THR  ((volatile unsigned char*)(UART0_BASE + 0))
#define UART0_LSR  ((volatile unsigned char*)(UART0_BASE + 5))

void uart_putc(char c) {
    while ((*UART0_LSR & (1 << 5)) == 0);
    *UART0_THR = c;
}

void uart_puts(const char *s) {
    while (*s) {
        uart_putc(*s++);
    }
}

int main(void) {
    uart_puts("Hello World!\n");
    while (1) {
        __asm__ volatile("wfi");
    }
    return 0;
}

