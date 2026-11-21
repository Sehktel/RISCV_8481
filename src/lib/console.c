#include "../include/console.h"
#include "../include/types.h"
#include "../include/uart.h"
#include <stdarg.h>


/**
 * @brief Печатает беззнаковое 64-битное число с дополнением ведущими нулями.
 *
 * @param num       Выводимое число.
 * @param base      Основание системы счисления: 10 или 16.
 * @param min_width Минимальное число выводимых цифр. Если число занимает
 *                  меньше разрядов, слева добавляются нули. 0 — без
 *                  дополнения.
 *
 * Буфер в 32 символа достаточен для худшего случая: 64 бита в base=10
 * дают максимум 20 цифр.
 *
 * @note Число с ведущими нулями при base=16 нельзя спутать со строкой:
 *       если нужен префикс "0x", его печатает вызывающий код (см. %p).
 */
void print_number(uint64_t num, int base, int min_width) {
    const char digits[] = "0123456789abcdef";
    char buf[32];
    int i = 0;

    if (num == 0) {
        buf[i++] = '0';
    } else {
        while (num > 0) {
            buf[i++] = digits[num % base];
            num /= base;
        }
    }

    // Дополняем ведущими нулями до минимальной ширины
    while (i < min_width && i < (int)sizeof(buf)) {
        buf[i++] = '0';
    }

    while (i > 0) {
        uart_putc(buf[--i]);
    }
}

/**
 * @brief Форматированный вывод в UART (bare-metal аналог printf).
 *
 * Поддержка: %c, %s, %d, %u, %x, %p, %%, модификатор длины 'l'.
 * '\n' автоматически разворачивается в CR+LF для терминала QEMU.
 * Ширины/точности (%5d, %.2f) не поддерживаются — печатаются буквально.
 *
 * @param fmt Строка формата.
 * @param ... Аргументы, строго соответствующие спецификаторам.
 */

void kprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt != '%') {
            if (*fmt == '\n') {
                uart_putc('\r');
            }
            uart_putc(*fmt++);
            continue;
        }

        fmt++;
        if (*fmt == '\0') break;

        // Флаг '0' — дополнение ведущими нулями (%08x)
        bool zero_pad = false;
        if (*fmt == '0') {
            zero_pad = true;
            fmt++;
            if (*fmt == '\0') break;
        }

        // Ширина поля — десятичные цифры (минимум цифр на выводе)
        int width = 0;
        while (*fmt >= '0' && *fmt <= '9') {
            width = width * 10 + (*fmt - '0');
            fmt++;
            if (*fmt == '\0') break;
        }
        if (*fmt == '\0') break;

        // Модификатор длины 'l'
        bool is_long = false;
        if (*fmt == 'l') {
            is_long = true;
            fmt++;
            if (*fmt == '\0') break;
        }

        // Для числовых форматов ширина превращается в дополнение нулями
        int min_width = zero_pad ? width : 0;

        switch (*fmt) {
        case 'c': {
            char c = (char)va_arg(args, int);
            uart_putc(c);
            break;
        }
        case 's': {
            const char* s = va_arg(args, const char*);
            if (!s) s = "(null)";
            print(s);
            break;
        }
        case 'd': {
            int64_t val = is_long ? (int64_t)va_arg(args, int64_t) : (int64_t)va_arg(args, int);
            if (val < 0) {
                uart_putc('-');
                print_number((uint64_t)(-(uint64_t)val), 10, min_width > 0 ? min_width - 1 : 0);
            } else {
                print_number((uint64_t)val, 10, min_width);
            }
            break;
        }
        case 'u': {
            uint64_t val = is_long ? va_arg(args, uint64_t) : (uint64_t)va_arg(args, unsigned int);
            print_number(val, 10, min_width);
            break;
        }
        case 'x': {
            uint64_t val = is_long ? va_arg(args, uint64_t) : (uint64_t)va_arg(args, unsigned int);
            print_number(val, 16, min_width);
            break;
        }
        case 'p': { // Указатель: "0x" + 16 hex-цифр (ровные колонки на RV64)
            uintptr_t ptr = (uintptr_t)va_arg(args, void*);
            print("0x");
            print_number((uint64_t)ptr, 16, 16);
            break;
        }
        case '%': {
            uart_putc('%');
            break;
        }
        default: {
            uart_putc('%');
            if (zero_pad) uart_putc('0');
            if (width > 0) print_number((uint64_t)width, 10, 0);
            if (is_long) uart_putc('l');
            uart_putc(*fmt);
            break;
        }
        }
        fmt++;
    }

    va_end(args);
}
