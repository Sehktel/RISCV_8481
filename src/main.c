// Регистры управления питанием QEMU (SiFive Test Finisher)
#define SYSCON_TEST_ADDR 0x100000
#define FINISHER_PASS    0x5555

// Адрес UART0 на плате QEMU virt
#define UART0_BASE       0x10000000
#define UART0_THR        ((volatile unsigned char*)(UART0_BASE + 0x00)) // Транспортный регистр (запись)
#define UART0_LSR        ((volatile unsigned char*)(UART0_BASE + 0x05)) // Статус линии

// Константы битов прерываний RISC-V
#define MSTATUS_MIE  (1ULL << 3)  // Глобальное разрешение прерываний в Machine-mode
#define MIE_MTIE     (1ULL << 7)  // Разрешение прерываний от машинного таймера

// Добавляем макрос для очистки битов в CSR регистрах
#define clear_csr(reg, bit) ({ asm volatile ("cclri " #reg ", %0" :: "r"(bit)); })
// Если компилятор ругается на cclri, используем универсальный вариант через чтение-запись:
#define write_csr(reg, val) ({ asm volatile ("csrw " #reg ", %0" :: "r"(val)); })
#define set_csr(reg, bit)   ({ asm volatile ("csrs " #reg ", %0" :: "r"(bit)); })

// Добавляем буффер данных для командной строки
// Для справки в Arduino почти такой же.
#define BUFFER_SIZE 64

// Добавляем стандартные заголовочные файлы для типов данных и функций
// Готовимся писать kprintf функцию, поэтому подключаем stdarg.h для работы с переменным числом аргументов
#include <stdarg.h>
#include <stdint.h>


void enable_interrupts(void) {
    // 1. Принудительно гасим ВСЕ разрешения прерываний (таймер, софтверные, внешние)
    // Записываем чистый 0 в регистр mie. Теперь ни одно устройство не побеспокоит процессор.
    write_csr(mie, 0);

    // 2. Сбрасываем (очищаем) все отложенные флаги прерываний в регистре mip.
    // Запись нуля в mip сбрасывает незавершенные аппаратные запросы, оставшиеся от загрузчика.
    write_csr(mip, 0);

    // 3. Теперь, когда периферия гарантированно молчит, безопасно включаем
    // архитектурную готовность ядра обрабатывать прерывания глобально.
    set_csr(mstatus, MSTATUS_MIE);
}

// Функция чтения одного символа (Busy-wait опрос регистра LSR)
char uart_getc(void) {
    // Ждем, пока бит 0 (Data Ready) станет равен 1
    while ((*UART0_LSR & (1 << 0)) == 0);
    // Возвращаем считанный символ из регистра данных (он же UART0_THR на чтение)
    return (char)(*UART0_THR);
}

// Вспомогательная функция для сравнения строк (bare-metal аналог strcmp)
int mystrcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

// Вспомогательная функция для определения длины строки
int mystrlen(const char *s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

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

// // Готовимся к дальнейшему переходу на doxygen
// /**
//  * @brief Функция обработки командной строки
//  * @param cmd - указатель на строку с командой, введенной пользователем
//  */
// void execute_command(const char *cmd) {

//     // Намеренная ошибка для проверки статического анализатора Cppcheck:
//     // Создаем массив на 5 элементов, а пишем в 10-й элемент (выход за границы буфера)
//     // int test_array[5];
//     // test_array[10] = 42;


//     if (mystrcmp(cmd, "help") == 0) {
//         print("Доступные команды:\n");
//         print("  help  - Вывод этого списка команд\n");
//         print("  clear - Очистить экран терминала\n");
//         print("  exit  - Завершение работы программы\n");
//     }
//     else if (mystrcmp(cmd, "clear") == 0) {
//         // ANSI escape-код: очистка экрана (\033[2J) и возврат курсора в левый верхний угол (\033[H)
//         print("\033[H\033[2J");
//     }
//     else if (mystrcmp(cmd, "exit") == 0) {
//         print("Завершение работы программы...\n");
//         poweroff();
//     }
//     else if (mystrlen(cmd) > 0) {
//         print("Ошибка: неизвестная команда. Введите 'help'\n");
//     }
// }


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

        kprintf("--- Диагностика ядра %s ---\n", "RISCV_8481");
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



/**
 * @brief Точка входа в ядро ОС \\
 * Почему void kmain(void) и не int main(void)? \\
 * потому что ядро -- это мы.
 */
void kmain(void) {

    // Включаем архитектурные прерывания процессора
   enable_interrupts();

    // Выводим приветствие
//    print("Hello World!\n");

    print("\n--- RISCV_8481 Микроядро: Shell загружен ---\n");
    print("sh> ");

    char buffer[BUFFER_SIZE];
    int index = 0;

    // Главный цикл ядра ОС
    while (1) {

        char c = uart_getc();

        // Обработка клавиши Enter (\r или \n в зависимости от настроек терминала хоста)
        if (c == '\r' || c == '\n') {
            buffer[index] = '\0'; // Замыкаем строку в буфере
            print("\n");          // Перенос строки на экране

            execute_command(buffer);

            index = 0;            // Сбрасываем индекс для новой команды
            print("sh> ");
        }
        // Обработка клавиши Backspace (ASCII 8 или 127)
        else if (c == 8 || c == 127) {
            if (index > 0) {
                index--;
                print("\b \b");   // Сдвигаем курсор назад, стираем пробелом, возвращаем курсор
            }
        }
        // Обычные печатаемые символы
        else {
            // Ограничиваем ввод размером буфера (оставляем 1 байт под '\0')
            if (index < BUFFER_SIZE - 1) {
                buffer[index++] = c;
                uart_putc(c);     // Эхо-вывод: отображаем символ пользователю при вводе
            }
        }
        // asm volatile("wfi"); -- хвост от прошлой жизни
    }


    poweroff();
}