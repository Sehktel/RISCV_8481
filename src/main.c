#include "console.h"
#include "csr.h"
#include "heap.h"
#include "kernel_layout.h"
#include "mm.h"
#include "pmp.h"
#include "sched.h"
#include "shell.h"
#include "string.h"
#include "timer.h"
#include "uart.h"

#include <stdarg.h>

#define BUFFER_SIZE 64

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
    csr_enable_interrupts();
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