#include "console.h"
#include "csr.h"
#include "heap.h"
#include "kernel_layout.h"
#include "mm.h"
#include "pmp.h"
#include "poweroff.h"
#include "sched.h"
#include "string.h"
#include "timer.h"
#include "uart.h"

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
    uint64_t total_ms = ticks / (TIMER_FREQ_HZ / 1000);

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
        kprintf("  sched        - Проверка вытесняющей многозадачности\n");
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
    } else if (kstrcmp(cmd, "sched") == 0) {
        run_scheduler_demo();
    } else if (kstrcmp(cmd, "pmptest") == 0) {
        run_pmp_violation_demo();
    } else if (kstrlen(cmd) > 0) {
        kprintf("Ошибка: неизвестная команда '%s'. Введите 'help'\n", cmd);
    }
}
