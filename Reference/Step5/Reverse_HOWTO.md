```c
// Добавляем стандартные заголовочные файлы для типов данных и функций
// Готовимся писать kprintf функцию, поэтому подключаем stdarg.h для работы с переменным числом аргументов
#include <stdarg.h>
#include <stdint.h>


// Вспомогательная функция печати беззнакового 64-битного числа в заданной системе счисления (base = 10 или 16)
void print_number(uint64_t num, int base) {
    const char digits[] = "0123456789abcdef";
    char buf[32]; // Буфер для обратного формирования цифр
    int i = 0;

    // Особый случай: если число равно 0
    if (num == 0) {
        uart_putc('0');
        return;
    }

    // Разбираем число на цифры с конца
    while (num > 0) {
        buf[i++] = digits[num % base];
        num /= base;
    }

    // Выводим цифры в правильном (прямом) порядке
    while (i > 0) {
        uart_putc(buf[--i]);
    }
}


// Полноценный bare-metal форматированный вывод
void kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt); // Инициализируем чтение аргументов после строки fmt

    while (*fmt) {
        // Если встретили обычный символ (не '%')
        if (*fmt != '%') {
            if (*fmt == '\n') {
                uart_putc('\r'); // Перенос строки для терминала QEMU
            }
            uart_putc(*fmt++);
            continue;
        }

        // Переходим к символу спецификатора сразу после '%'
        fmt++;
        if (*fmt == '\0') break; // Защита от обрыва строки на символе '%'

        switch (*fmt) {
            case 'c': { // Одиночный символ
                char c = (char)va_arg(args, int);
                uart_putc(c);
                break;
            }
            case 's': { // Строка
                const char *s = va_arg(args, const char *);
                if (!s) s = "(null)";
                print(s);
                break;
            }
            case 'd': { // Знаковое целое число (десятичное)
                int val = va_arg(args, int);
                if (val < 0) {
                    uart_putc('-');
                    print_number((uint64_t)(-val), 10);
                } else {
                    print_number((uint64_t)val, 10);
                }
                break;
            }
            case 'x': { // Беззнаковое шестнадцатеричное число (удобно для адресов памяти)
                uint64_t val = va_arg(args, uint64_t);
                print_number(val, 16);
                break;
            }
            case '%': { // Экранированный знак процента '%%'
                uart_putc('%');
                break;
            }
            default: { // Неизвестный спецификатор печатаем как есть
                uart_putc('%');
                uart_putc(*fmt);
                break;
            }
        }
        fmt++;
    }

    va_end(args); // Очищаем список аргументов
}

void execute_command(const char *cmd) {

    // Намеренная ошибка для проверки статического анализатора Cppcheck:
    // Создаем массив на 5 элементов, а пишем в 10-й элемент (выход за границы буфера)
    // int test_array[5];
    // test_array[10] = 42;

    if (mystrcmp(cmd, "help") == 0) {
        kprintf("Доступные команды:\n");
        kprintf("  help  - Вывод этого списка команд\n");
        kprintf("  info  - Вывод системной информации ядра\n");
        kprintf("  clear - Очистить экран терминала\n");
        kprintf("  exit  - Завершение работы программы\n");
    }
    else if (mystrcmp(cmd, "info") == 0) {
        uint64_t sp_val;
        // Читаем текущий указатель стека sp напрямую из регистра процессора
        asm volatile("mv %0, sp" : "=r"(sp_val));

        kprintf("--- Диагностика ядра %s ---\n", "RISCV");
        kprintf("Архитектура: RV%dGC (64-bit)\n", 64);
        kprintf("Базовый адрес UART0: 0x%x\n", (uint64_t)UART0_BASE);
        kprintf("Текущий указатель стека (SP): 0x%x\n", sp_val);
    }
    else if (mystrcmp(cmd, "clear") == 0) {
        kprintf("\033[H\033[2J");
    }
    else if (mystrcmp(cmd, "exit") == 0) {
        kprintf("Завершение работы системы...\n");
        poweroff();
    }
    else if (mystrlen(cmd) > 0) {
        kprintf("Ошибка: неизвестная команда '%s'. Введите 'help'\n", cmd);
    }
}
```
