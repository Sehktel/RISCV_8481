// Регистры управления питанием QEMU (SiFive Test Finisher)
#define SYSCON_TEST_ADDR 0x100000
#define FINISHER_PASS    0x5555

// Адрес UART0 на плате QEMU virt
#define UART0_BASE       0x10000000
#define UART0_THR        ((volatile unsigned char*)(UART0_BASE + 0x00)) // Транспортный регистр (запись)
#define UART0_LSR        ((volatile unsigned char*)(UART0_BASE + 0x05)) // Статус линии

// Функция вывода одного символа в UART
void uart_putc(char c) {
    // Ждем, пока освободится буфер передачи (статус-бит THRE = 5-й бит)
    while ((*UART0_LSR & (1 << 5)) == 0);
    // Пишем символ в регистр данных
    *UART0_THR = c;
}

// Простая функция вывода строки
void print(const char *str) {
    while (*str) {
        if (*str == '\n') {
            uart_putc('\r'); // Для терминалов nographic перенос строки требует CR+LF
        }
        uart_putc(*str++);
    }
}

// Функция чистого выключения эмулятора QEMU
void poweroff(void) {
    volatile unsigned int *test_finisher = (volatile unsigned int *)SYSCON_TEST_ADDR;
    *test_finisher = FINISHER_PASS;

    // Предохранитель, если QEMU не отреагировал
    while (1) {
        __asm__ volatile("wfi");
    }
}

int main(void) {
    print("Hello World!\n");

    // Автоматически тушим QEMU после завершения работы программы
    poweroff();

    return 0;
}
