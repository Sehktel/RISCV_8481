
/**
 * @file kernel.c
 * @brief Минимальное микроядро RISC-V (RV64GC) для QEMU virt с интерактивным shell.
 *
 * Реализует UART-ввод/вывод, форматированный вывод (kprintf), чтение
 * аппаратного таймера и обработку команд пользователя. Сборка — bare-metal,
 * без стандартной библиотеки.
 */

#include "main.h"

#include <stdarg.h>

/**
 * @brief Инициализирует кучу: создаёт один большой свободный блок.
 *
 * Берёт границы кучи из символов компоновщика @c _heap_start и
 * @c _heap_end, выравнивает их по 16 байт и превращает всю область
 * в единственный свободный блок.
 *
 * @warning Функцию нужно вызвать один раз до любого kmalloc/kfree.
 *          При слишком маленькой области (меньше одного заголовка)
 *          куча остаётся неинициализированной (heap_head == NULL),
 *          и kmalloc будет возвращать NULL.
 */
void heap_init(void) {
    heap_bounds_start = ALIGN16((uintptr_t)_heap_start);
    heap_bounds_end = (uintptr_t)_heap_end & ~((uintptr_t)15);

    if (heap_bounds_end <= heap_bounds_start + sizeof(block_header_t)) {
        heap_head = NULL;
        return;
    }

    heap_head = (block_header_t*)heap_bounds_start;
    heap_head->magic = HEAP_MAGIC_FREE;
    heap_head->size = (size_t)(heap_bounds_end - heap_bounds_start - sizeof(block_header_t));
    heap_head->is_free = 1;
    heap_head->next = NULL;
    // Отравляем весь payload, чтобы инвариант "свободное = 0xA5"
    // выполнялся с самого начала
    uint8_t* raw = (uint8_t*)heap_head + sizeof(block_header_t);
    for (size_t i = 0; i < heap_head->size; i++)
        raw[i] = HEAP_POISON_BYTE;
}

/**
 * @brief Выводит карту кучи: все блоки, их адреса, размеры и статус.
 *
 * Параллельно подсчитывает суммарный объём занятой и свободной памяти
 * (без учёта заголовков) и печатает итог. Удобно для отладки фрагментации
 * и проверки коалесценции после kfree.
 */
void heap_dump(void) {
    block_header_t* curr = heap_head;
    size_t total_free = 0;
    size_t total_used = 0;
    size_t block_idx = 0;

    kprintf("=== Карта кучи (Heap Map) ===\n");
    while (curr != NULL) {
        // Проверяем целостность заголовка при обходе
        const char* status = "CORRUPTED!";
        if (curr->magic == HEAP_MAGIC_FREE && curr->is_free) {
            status = "FREE";
            total_free += curr->size;
        } else if (curr->magic == HEAP_MAGIC_ALLOC && !curr->is_free) {
            status = "USED";
            total_used += curr->size;
        }

        kprintf("[%d] Block: 0x%x | Payload: 0x%x | Size: %d B | %s\n", (int)block_idx, (uintptr_t)curr,
                (uintptr_t)curr + sizeof(block_header_t), (int)curr->size, status);

        curr = curr->next;
        block_idx++;
    }
    kprintf("Всего блоков: %d | Занято: %d B | Свободно: %d B\n", (int)block_idx, (int)total_used, (int)total_free);
}

/**
 * @brief Нагрузочный тест аллокатора с демонстрацией коалесценции.
 *
 * Сценарий:
 * 1. Выделяются три блока (64, 128, 256 байт).
 * 2. Освобождается средний блок — в куче образуется «дыра».
 * 3. Освобождается первый блок — он должен слиться со средним (коалесценция
 *    с последующим соседом).
 * 4. Освобождается третий блок — вся куча должна свернуться в один
 *    свободный блок.
 */
void heap_test(void) {
    kprintf("=== 1. Тест выделения и коалесценции ===\n");
    void* p1 = kmalloc(64);
    void* p2 = kmalloc(128);
    void* p3 = kmalloc(256);
    heap_dump();

    kprintf("\n--- Освобождаем средний блок (p2) ---\n");
    kfree(p2);
    heap_dump();

    kprintf("\n--- Освобождаем p1 (слияние вперёд с p2) ---\n");
    kfree(p1);
    heap_dump();

    kprintf("\n--- Освобождаем p3 (слияние назад, куча должна стать единой) ---\n");
    kfree(p3);
    heap_dump();

    kprintf("\n=== 2. Тест безопасности (Security Checks) ===\n");
    kprintf("Попытка Double-Free на p3:\n");
    kfree(p3);

    kprintf("Попытка освободить невыровненный указатель:\n");
    kfree((void*)0x80005003);

    kprintf("Попытка освободить указатель вне кучи (например, из стека/кода):\n");
    kfree((void*)0x80000010);
}

/**
 * @brief Выделяет блок памяти размером @p size байт (first-fit поиск).
 * @param size Запрашиваемый размер в байтах; 0 возвращает NULL.
 * @return Указатель на полезную нагрузку блока или NULL, если
 *         подходящий блок не найден (куча исчерпана или не инициализирована).
 *
 * Алгоритм:
 * 1. Размер выравнивается вверх до 16 байт.
 * 2. Список просматривается до первого свободного блока достаточного размера.
 * 3. Если в блоке остаётся запас минимум на заголовок + 16 байт полезной
 *    нагрузки, блок расщепляется: «хвост» становится новым свободным блоком.
 * 4. Иначе блок отдаётся целиком — пользователь получает больше,
 *    чем просил (внутренняя фрагментация).
 *
 * @note Сложность O(n) по числу блоков. Для небольшого ядра достаточно;
 *       при росте имеет смысл вести отдельный список свободных блоков.
 */
void* kmalloc(size_t size) {
    if (size == 0 || heap_head == NULL) return NULL;

    if (size > (size_t)-1 - sizeof(block_header_t) - 16) {
        kprintf("[SECURITY] kmalloc: переполнение размера, запрос отклонён\n");
        return NULL;
    }

    size = ALIGN16(size);
    block_header_t* curr = heap_head;

    while (curr != NULL) {
        // Проверка целостности кучи на лету (защита от повреждения метаданных)
        if (curr->magic != HEAP_MAGIC_FREE && curr->magic != HEAP_MAGIC_ALLOC) {
            kprintf("[PANIC] Heap corruption detected at 0x%x!\n", (uintptr_t)curr);
            for (;;) {
                __asm__ volatile("wfi");
            } // куча битая -- продолжать опасно -- бесконечный цикл с wfi
        }

        if (curr->is_free && curr->size >= size) {
            const uint8_t* probe = (const uint8_t*)curr + sizeof(block_header_t);
            for (int i = 0; i < HEAP_UAF_CHECK_BYTES; i++) {
                if (probe[i] != HEAP_POISON_BYTE) {
                    kprintf("[SECURITY] Use-after-free detected at 0x%x (payload перезаписан после kfree)\n",
                            (uintptr_t)curr);
                    break;
                }
            }

            if (curr->size >= size + sizeof(block_header_t) + 16) {
                block_header_t* new_block = (block_header_t*)((uint8_t*)curr + sizeof(block_header_t) + size);
                new_block->magic = HEAP_MAGIC_FREE;
                new_block->size = curr->size - size - sizeof(block_header_t);
                new_block->is_free = 1;
                new_block->next = curr->next;

                curr->size = size;
                curr->next = new_block;
            }
            curr->is_free = 0;
            curr->magic = HEAP_MAGIC_ALLOC;
            return (void*)((uint8_t*)curr + sizeof(block_header_t));
        }
        curr = curr->next;
    }
    return NULL;
}

/**
 * @brief Освобождает ранее выделенный блок и сливает его со свободными соседями.
 * @param ptr Указатель, возвращённый kmalloc (NULL игнорируется).
 *
 * Алгоритм:
 * 1. Проверяет валидность указателя (внутри границ кучи, выровнен по 16).
 * 2. Помечает блок свободным.
 * 3. Сливает блок с последующим соседом, если тот свободен.
 * 4. Находит предыдущий блок и сливает с ним, если он свободен.
 *
 * Защита от невалидных указателей: при выходе за границы кучи или
 * нарушении выравнивания печатает ошибку и не трогает память.
 *
 * @note Двойной kfree одного указателя этой проверкой не ловится —
 *       для полного контроля добавьте в заголовок магическое поле-страж
 *       (см. замечания ниже).
 */
void kfree(void* ptr) {
    if (ptr == NULL) return;

    uintptr_t addr = (uintptr_t)ptr;

    // 1. Проверка выравнивания указателя
    if ((addr & 15) != 0) {
        kprintf("[SECURITY] kfree: unaligned pointer 0x%x\n", addr);
        return;
    }

    // 2. Проверка попадания указателя в границы кучи
    if (addr < heap_bounds_start + sizeof(block_header_t) || addr >= heap_bounds_end) {
        kprintf("[SECURITY] kfree: pointer out of heap bounds 0x%x\n", addr);
        return;
    }

    block_header_t* block = (block_header_t*)(addr - sizeof(block_header_t));

    // 3. Проверка магического числа (ловит мусорные указатели и Double-Free)
    if (block->magic == HEAP_MAGIC_FREE || block->is_free) {
        kprintf("[SECURITY] kfree: Double-Free detected at 0x%x!\n", addr);
        return;
    }
    if (block->magic != HEAP_MAGIC_ALLOC) {
        kprintf("[SECURITY] kfree: Invalid magic (corrupted/foreign ptr) at 0x%x!\n", addr);
        return;
    }

    // Помечаем блок как свободный
    block->is_free = 1;
    block->magic = HEAP_MAGIC_FREE;

    // 4. Локальная коалесценция ВПЕРЁД (со следующим блоком)
    if (block->next != NULL && block->next->is_free && block->next->magic == HEAP_MAGIC_FREE) { //
        block_header_t* victim = block->next;
        size_t victim_size = victim->size;          // : сохраняем ПОЛЕ
        block_header_t* victim_next = victim->next; // : ДО затирания

        // вместо magic=0 пишем яд — заголовок становится
        // частью свободного payload, инвариант сохраняется
        uint8_t* raw = (uint8_t*)victim;
        for (size_t i = 0; i < sizeof(block_header_t); i++)
            raw[i] = HEAP_POISON_BYTE;

        block->size += sizeof(block_header_t) + victim_size;
        block->next = victim_next;

        // 5. Локальная коалесценция НАЗАД (поиск предыдущего блока)
        block_header_t* blk_next = block->next; // сохраняем next до затирания
        block_header_t* prev = heap_head;
        while (prev != NULL && prev->next != block) {
            prev = prev->next;
        }
        if (prev != NULL && prev->is_free && prev->magic == HEAP_MAGIC_FREE) { //
            prev->size += sizeof(block_header_t) + block->size;

            // Заголовок тоже становится payload — ядовит
            uint8_t* raw = (uint8_t*)block;
            for (size_t i = 0; i < sizeof(block_header_t); i++)
                raw[i] = HEAP_POISON_BYTE;

            prev->next = blk_next;
        }
    }
}

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
 * @brief Читает один символ из UART методом активного опроса (busy-wait).
 * @return Принятый символ (char).
 *
 * Крутится в цикле, пока бит Data Ready (LSR[0]) не станет равен 1,
 * затем возвращает символ из регистра данных.
 */
char uart_getc(void) {
    // Ждем, пока бит 0 (Data Ready) станет равен 1
    while ((*UART0_LSR & (1 << 0)) == 0)
        ;
    // Возвращаем считанный символ из регистра данных (он же UART0_THR на чтение)
    return (char)(*UART0_THR);
}

/**
 * @brief Сравнивает две NUL-терминированные строки (bare-metal аналог strcmp).
 * @param s1 Указатель на первую строку.
 * @param s2 Указатель на вторую строку.
 * @return 0, если строки равны; разница кодов первых различающихся символов
 *         в противном случае (положительная, если s1 > s2).
 * @note Досрочный выход происходит, когда *s1 == 0 или символы различны,
 *       поэтому корректно сравнивает строки разной длины.
 */
int mystrcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

/**
 * @brief Вычисляет длину NUL-терминированной строки.
 * @param s Указатель на строку.
 * @return Количество символов в строке без учёта завершающего '\0'.
 */
int mystrlen(const char* s) {
    int len = 0;
    while (s[len])
        len++;
    return len;
}

/**
 * @brief Выводит один символ в UART.
 * @param c Символ для передачи.
 *
 * Перед записью ждёт освобождения передатчика: бит THRE (LSR[5])
 * должен быть равен 1.
 */
void uart_putc(char c) {
    while ((*UART0_LSR & (1 << 5)) == 0)
        ; // Ожидание свободного буфера передачи
    *UART0_THR = c;
}

/**
 * @brief Выводит NUL-терминированную строку в UART.
 * @param str Указатель на строку (не NULL).
 *
 * Символ '\n' автоматически заменяется парой CR+LF ('\r' + '\n'),
 * как требует терминал QEMU в режиме nographic.
 */
void print(const char* str) {
    while (*str) {
        if (*str == '\n') {
            uart_putc('\r'); // Для терминалов nographic перенос строки требует CR+LF
        }
        uart_putc(*str++);
    }
}

/**
 * @brief Выполняет чистое выключение эмулятора QEMU.
 *
 * Записывает "магическое" значение @c FINISHER_PASS (0x5555) в устройство
 * sifive_test по адресу @c SYSCON_TEST_ADDR — QEMU завершает работу с кодом 0.
 *
 * @note На реальном железе этот адрес ничего не отключит: предусмотрена
 *       страховка — бесконечный цикл с инструкцией wfi (wait for interrupt).
 */
void poweroff(void) {
    volatile unsigned int* test_finisher = (volatile unsigned int*)SYSCON_TEST_ADDR;
    *test_finisher = FINISHER_PASS;

    // Предохранитель, если QEMU не отреагировал
    while (1) {
        __asm__ volatile("wfi");
    }
}

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
static void print_number(uint64_t num, int base, int min_width) {
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

/**
 * @brief Читает 64-битный счётчик аппаратного времени (mtime).
 * @return Текущее значение счётчика в тиках.
 *
 * В QEMU virt тики идут с частотой TIMER_FREQ (10 МГц).
 * Инструкция rdtime атомарно читает CSR time, так что разрыв
 * между старшим и младшим словом невозможен.
 */
uint64_t get_ticks(void) {
    uint64_t ticks;
    asm volatile("rdtime %0" : "=r"(ticks));
    return ticks;
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
 * | exit, quit    | Завершение работы (poweroff)                      |
 *
 * Пустая строка молча игнорируется; неизвестная команда печатает ошибку.
 */
void execute_command(const char* cmd) {

    // Намеренная ошибка для проверки статического анализатора Cppcheck:
    // Создаем массив на 5 элементов, а пишем в 10-й элемент (выход за границы буфера)
    // int test_array[5];
    // test_array[10] = 42;

    if (mystrcmp(cmd, "help") == 0) {
        kprintf("Доступные команды:\n");
        kprintf("  help         - Вывод этого списка команд\n");
        kprintf("  info         - Вывод системной информации ядра\n");
        kprintf("  clear        - Очистить экран терминала\n");
        kprintf("  time, uptime - Показать время работы системы\n");
        kprintf("  mem          - Показать карту кучи (heap)\n");
        kprintf("  memtest      - Выполнить тест аллокатора памяти\n");
        kprintf("  exit, quit   - Завершение работы программы\n");
    } else if (mystrcmp(cmd, "info") == 0) {
        uint64_t sp_val;
        // Читаем текущий указатель стека sp напрямую из регистра процессора
        asm volatile("mv %0, sp" : "=r"(sp_val));

        kprintf("--- Диагностика ядра %s ---\n", "RISCV");
        kprintf("Архитектура: RV%dGC (64-bit)\n", 64);
        kprintf("Базовый адрес UART0: 0x%x\n", (uint64_t)UART0_BASE);
        kprintf("Текущий указатель стека (SP): 0x%x\n", sp_val);
    } else if (mystrcmp(cmd, "clear") == 0) {
        kprintf("\033[H\033[2J");
    } else if (mystrcmp(cmd, "exit") == 0 || mystrcmp(cmd, "quit") == 0) {
        kprintf("Завершение работы системы...\n");
        poweroff();
    } else if (mystrcmp(cmd, "uptime") == 0 || mystrcmp(cmd, "time") == 0) {
        print_uptime();
    } else if (mystrcmp(cmd, "mem") == 0) {
        heap_dump();
    } else if (mystrcmp(cmd, "memtest") == 0) {
        heap_test();
    } else if (mystrlen(cmd) > 0) {
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

    // Выводим приветствие
    //    print("Hello World!\n");

    print("\n--- RISCV Микроядро: Shell загружен ---\n");
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

            index = 0; // Сбрасываем индекс для новой команды
            print("sh> ");
        }
        // Обработка клавиши Backspace (ASCII 8 или 127)
        else if (c == 8 || c == 127) {
            if (index > 0) {
                index--;
                print("\b \b"); // Сдвигаем курсор назад, стираем пробелом, возвращаем курсор
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