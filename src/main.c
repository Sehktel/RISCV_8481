// Регистры управления питанием QEMU (SiFive Test Finisher)
#define SYSCON_TEST_ADDR 0x100000
#define FINISHER_PASS    0x5555

// Адрес UART0 на плате QEMU virt
#define UART0_BASE       0x10000000
#define UART0_THR        ((volatile unsigned char*)(UART0_BASE + 0x00)) // Транспортный регистр (запись)
#define UART0_LSR        ((volatile unsigned char*)(UART0_BASE + 0x05)) // Статус линии

// Добавляем макросы для записи в CSR регистры
#define write_csr(reg, val) ({ asm volatile ("csrw " #reg ", %0" :: "r"(val)); })
#define set_csr(reg, bit)   ({ asm volatile ("csrs " #reg ", %0" :: "r"(bit)); })

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

    // // 1. Разрешаем прерывания конкретно от таймера (Machine Timer Interrupt)
    // set_csr(mie, MIE_MTIE);

    // // 2. Включаем прерывания глобально
    // set_csr(mstatus, MSTATUS_MIE);
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

// Готовимся к дальнейшему переходу на doxygen
/**
 * @brief Функция обработки командной строки
 * @param cmd - указатель на строку с командой, введенной пользователем
 */
void execute_command(const char *cmd) {
    if (mystrcmp(cmd, "help") == 0) {
        print("Доступные команды:\n");
        print("  help  - Вывод этого списка команд\n");
        print("  clear - Очистить экран терминала\n");
        print("  exit  - Завершение работы программы\n");
    }
    else if (mystrcmp(cmd, "clear") == 0) {
        // ANSI escape-код: очистка экрана (\033[2J) и возврат курсора в левый верхний угол (\033[H)
        print("\033[H\033[2J");
    }
    else if (mystrcmp(cmd, "exit") == 0) {
        print("Завершение работы программы...\n");
        poweroff();
    }
    else if (mystrlen(cmd) > 0) {
        print("Ошибка: неизвестная команда. Введите 'help'\n");
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