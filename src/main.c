
/**
 * @file kernel.c
 * @brief Минимальное микроядро RISC-V (RV64GC) для QEMU virt с интерактивным shell.
 *
 * Реализует UART-ввод/вывод, форматированный вывод (kprintf), чтение
 * аппаратного таймера и обработку команд пользователя. Сборка — bare-metal,
 * без стандартной библиотеки.
 */

#include "./include/console.h"
#include "./include/csr.h"
#include "./include/heap.h"
#include "./include/kernel_layout.h"
#include "./include/mm.h"
#include "./include/pmp.h"
#include "./include/sched.h"
#include "./include/string.h"
#include "./include/timer.h"
#include "./include/uart.h"

#include <stdarg.h>

#define BUFFER_SIZE 64

#define MSTATUS_MIE (1ULL << 3) // Глобальное разрешение прерываний в Machine-mode
#define MIE_MTIE (1ULL << 7)    // Разрешение прерываний от машинного таймера

/**
 * @brief Включает архитектурные прерывания процессора в Machine-mode.
 *
 * Последовательность инициализации:
 * 1. Полностью очищает регистр @c mie — ни одно устройство не побеспокоит ядро.
 * 2. Сбрасывает отложенные флаги в @c mip, оставшиеся от загрузчика.
 * 3. Устанавливает глобальный бит @c MIE в @c mstatus.
 *
 * @warning Регистр обработчика прерываний (mtvec) не настраивается здесь —
 *          до его инициализации включать MSTATUS_MIE опасно.
 */
void enable_interrupts(void) {

    /**

    Структура основных битов регистра mip

    Стандартные биты соответствуют типам программных, таймерных и внешних прерываний для разных уровней привилегий
    (Machine и Supervisor):

    Бит	Поле	Описание	Доступность / Запись
    1	SSIP	Supervisor Software Interrupt Pending (прерывание программного обеспечения уровня Supervisor)	Чтение /
    Запись (при наличии режима S) 3	MSIP	Machine Software Interrupt Pending (прерывание программного обеспечения
    уровня Machine)	Чтение / Запись 5	STIP	Supervisor Timer Interrupt Pending (таймерное прерывание уровня
    Supervisor)	Чтение / Запись (или управляется таймером) 7	MTIP	Machine Timer Interrupt Pending (таймерное
    прерывание уровня Machine)	Обычно чтение (устанавливается таймером M-mode, например CLINT) 9	SEIP	Supervisor
    External Interrupt Pending (внешнее прерывание уровня Supervisor)	Чтение / Запись или управляется PLIC/APLIC 11
    MEIP	Machine External Interrupt Pending (внешнее прерывание уровня Machine)	Обычно чтение (устанавливается
    контроллером прерываний)

    Примечание: В расширенных спецификациях (например, с поддержкой LCOFI или виртуализации) могут присутствовать и
    другие биты.

    Как это работает на практике

    1. Когда периферийное устройство, таймер или программный триггер вызывают прерывание, соответствующий бит в регистре
    mip устанавливается в 1.
    2. Чтобы прерывание реально обработалось процессором, должны совпасть три условия:
        • Глобальное разрешение прерываний в регистре mstatus (бит MIE для M-mode).
        • Соответствующий бит разрешения в регистре mie должен быть равен 1.
        • Бит ожидания в регистре mip должен быть равен 1.
    Подробную спецификацию привилегированного уровня можно найти в официальной библиотеке RISC-V Privileged
    Architecture.

    */

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

/**
 * @brief Вычисляет и выводит время работы ядра в формате MM:SS.mmm.
 *
 * Аппаратные тики (rdtime) переводятся в миллисекунды делением на
 * TIMER_FREQ / 1000. Часы не отображаются, только минуты и секунды.
 * В конце печатается счётчик тиков (при больших аптаймах приводит
 * к переполнению при касте к int — чисто косметика для отладки).
 */
void print_uptime(void) {
    uint64_t ticks = get_ticks();

    // Переводим тики в миллисекунды
    uint64_t total_ms = ticks / (TIMER_FREQ / 1000);

    uint64_t ms = total_ms % 1000;
    uint64_t seconds = (total_ms / 1000) % 60;
    uint64_t minutes = (total_ms / 60000);

    kprintf("Uptime: ");
    if (minutes < 10) uart_putc('0');
    kprintf("%d:", (int)minutes);

    if (seconds < 10) uart_putc('0');
    kprintf("%d.", (int)seconds);

    if (ms < 100) uart_putc('0');
    if (ms < 10) uart_putc('0');
    kprintf("%d (Ticks: %d)\n", (int)ms, (int)ticks);
}

/**
 * @brief Злонамеренная задача U-mode: пытается напрямую писать в UART0 в обход syscall.
 */
static void rogue_uart_task(void) {
    sys_call(SYS_PRINT, (uint64_t) "[U-MODE | ROGUE] Пытаюсь напрямую записать символ в UART0 (0x10000000)...\n", 0);
    *(volatile uint8_t*)UART0_BASE = 'X'; /* осознанный обход API: должен дать Store Access Fault, mcause=7 */
    sys_call(SYS_PRINT, (uint64_t) "[U-MODE | ROGUE] ОШИБКА: запись в UART0 прошла без блокировки!\n", 0);
    sys_call(SYS_EXIT, 0, 0);
}
void run_pmp_violation_demo(void) {
    const size_t user_stack_size = 4096;
    uint8_t* user_stack = (uint8_t*)kmalloc(user_stack_size);
    if (user_stack == NULL) {
        kprintf("[ERROR] Не удалось выделить память под стек U-mode\n");
        return;
    }

    /* Запоминаем, куда вернуться при любой ловушке из U-mode (SYS_EXIT,
     * PMP-фолт и т.д.): kill_current_task и exit_user_mode_return обе
     * используют shell_return_sp. Фрейм enter_user_mode — 112 байт. */
    uint64_t cur_sp;
    asm volatile("mv %0, sp" : "=r"(cur_sp));
    sched_set_shell_return_sp(cur_sp - 112);
    // shell_return_sp = cur_sp - 112;

    /* Планировщик тут не при делах: демо вне задач, поэтому снимаем
     * контекст "текущей задачи", чтобы kill_current_task ничего не убил. */
    sched_clear_current_task(); // current_task_idx = -1;

    uintptr_t user_sp = (uintptr_t)(user_stack + user_stack_size);
    pmp_configure_user((uintptr_t)user_stack, user_sp); /* строгий профиль */

    kprintf("[KERNEL] Запускаем задачу-нарушитель U-mode со строгим PMP...\n");
    enter_user_mode((uintptr_t)rogue_uart_task, user_sp);

    /* Сюда попадаем двумя путями: rogue штатно вернулся через SYS_EXIT
     * либо был убит по PMP-фолту — оба идут через shell_return_sp. */
    kprintf("[KERNEL] Ядро устояло, управление возвращено в M-mode Shell!\n");

    /* Восстанавливаем дефолтный профиль защиты: строгий PMP этого демо
     * не должен влиять на последующие запуски (например, shed). */
    pmp_set_default_user_profile();

    kfree(user_stack);
}

/**
 * @brief Разбирает и выполняет команду, введённую пользователем.
 * @param cmd Указатель на NUL-терминированную строку с командой.
 *
 * Поддерживаемые команды:
 * | Команда       | Действие                                          |
 * |---------------|---------------------------------------------------|
 * | help          | Вывод списка команд                               |
 * | info          | Диагностика ядра (архитектура, UART, SP)          |
 * | clear         | Очистка экрана (ANSI ESC[2J + ESC[H)              |
 * | time, uptime  | Время работы системы                              |
 * | mem           | Показать карту кучи (heap)                        |
 * | memtest       | Выполнить тест аллокатора памяти                  |
 * | usermode      | Запустить задачу в User Mode (U-mode + ecall)     |
 * | sched         | Проверка вытесняющей многозадачности              |
 * | pmptest       | Проверка аппаратная защита PMP                    |
 * | exit, quit    | Завершение работы (poweroff)                      |
 *
 * Пустая строка молча игнорируется; неизвестная команда печатает ошибку.
 */
void execute_command(const char* cmd) {

    // Намеренная ошибка для проверки статического анализатора Cppcheck:
    // Создаем массив на 5 элементов, а пишем в 10-й элемент (выход за границы буфера)
    // int test_array[5];
    // test_array[10] = 42;

    if (kstrcmp(cmd, "help") == 0) {
        kprintf("Доступные команды:\n");
        kprintf("  help         - Вывод этого списка команд\n");
        kprintf("  info         - Вывод системной информации ядра\n");
        kprintf("  clear        - Очистить экран терминала\n");
        kprintf("  time, uptime - Показать время работы системы\n");
        kprintf("  mem          - Показать карту кучи (heap)\n");
        kprintf("  memtest      - Выполнить тест аллокатора памяти\n");
        kprintf("  usermode     - Запустить задачу в User Mode (U-mode + ecall)\n");
        kprintf("  shed         - Проверка вытесняющей многозадачности\n");
        kprintf("  pmptest      - Проверка аппаратной защита PMP\n");
        kprintf("  exit, quit   - Завершение работы программы\n");
    } else if (kstrcmp(cmd, "info") == 0) {
        uint64_t sp_val;
        // Читаем текущий указатель стека sp напрямую из регистра процессора
        asm volatile("mv %0, sp" : "=r"(sp_val));

        kprintf("--- Диагностика ядра %s ---\n", "RISCV");
        kprintf("Архитектура: RV%dGC (64-bit)\n", 64);
        kprintf("Базовый адрес UART0: 0x%x\n", (uint64_t)UART0_BASE);
        kprintf("Текущий указатель стека (SP): 0x%x\n", sp_val);
    } else if (kstrcmp(cmd, "clear") == 0) {
        kprintf("\033[H\033[2J");
    } else if (kstrcmp(cmd, "exit") == 0 || kstrcmp(cmd, "quit") == 0) {
        kprintf("Завершение работы системы...\n");
        poweroff();
    } else if (kstrcmp(cmd, "uptime") == 0 || kstrcmp(cmd, "time") == 0) {
        print_uptime();
    } else if (kstrcmp(cmd, "mem") == 0) {
        heap_dump();
    } else if (kstrcmp(cmd, "memtest") == 0) {
        heap_test();
    } else if (kstrcmp(cmd, "usermode") == 0) {
        run_user_mode_demo();
    } else if (kstrcmp(cmd, "shed") == 0) {
        run_scheduler_demo();
    } else if (kstrcmp(cmd, "pmptest") == 0) {
        run_pmp_violation_demo();
    } else if (kstrlen(cmd) > 0) {
        kprintf("Ошибка: неизвестная команда '%s'. Введите 'help'\n", cmd);
    }
}

/**
 * @brief Точка входа в ядро ОС.
 *
 * Почему void kmain(void), а не int main(void)? Потому что ядро — это мы:
 * стандартная libc-обвязка и точка возврата в "программу" не существуют.
 *
 * Алгоритм работы:
 * 1. Включает прерывания.
 * 2. Печатает баннер и приглашение "sh> ".
 * 3. В бесконечном цикле читает символы из UART, обрабатывая:
 *    - Enter ('\r' или '\n') — выполнить накопленную команду;
 *    - Backspace (8 или 127) — стереть последний символ;
 *    - печатаемые символы — добавить в буфер с эхо-выводом.
 *
 * @warning Буфер ограничен BUFFER_SIZE-1 символами, лишние молча отбрасываются.
 * @note Функция не возвращает управление: единственный путь из цикла — poweroff().
 */
void kmain(void) {
    // uart_init();
    heap_init(); // <-- Обязательно до запуска шелла!

    // Включаем архитектурные прерывания процессора
    enable_interrupts();

    kprintf("\n--- RISCV Микроядро: Shell загружен ---\n");
    kprintf("sh> ");

    char buffer[BUFFER_SIZE];
    int index = 0;

    // Главный цикл ядра ОС
    while (true) { // while (true) более правильно чем while(1)

        char c = uart_getc();

        // Обработка клавиши Enter (\r или \n в зависимости от настроек терминала хоста)
        if (c == '\r' || c == '\n') {
            buffer[index] = '\0'; // Замыкаем строку в буфере
            kprintf("\n");        // Перенос строки на экране

            execute_command(buffer);

            index = 0; // Сбрасываем индекс для новой команды
            kprintf("sh> ");
        }
        // Обработка клавиши Backspace (ASCII 8 или 127)
        else if (c == 8 || c == 127) {
            if (index > 0) {
                index--;
                kprintf("\b \b"); // Сдвигаем курсор назад, стираем пробелом, возвращаем курсор
            }
        }
        // Обычные печатаемые символы
        else {
            // Ограничиваем ввод размером буфера (оставляем 1 байт под '\0')
            if (index < BUFFER_SIZE - 1) {
                buffer[index++] = c;
                uart_putc(c); // Эхо-вывод: отображаем символ пользователю при вводе
            }
        }
    }

    poweroff(); // Недостижимо, и это правильно.
}