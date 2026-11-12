```c
/// Частота таймера: 10 МГц для QEMU virt, 25 МГц для реального железа
#define TIMER_FREQ 10000000ULL

```

```c
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

    uint64_t ms      = total_ms % 1000;
    uint64_t seconds = (total_ms / 1000) % 60;
    uint64_t minutes = (total_ms / 60000);

    kprintf("Uptime: ");
    if (minutes < 10) uart_putc('0');
    kprintf("%d:", (int)minutes);

    if (seconds < 10) uart_putc('0');
    kprintf("%d.", (int)seconds);

    if (ms < 100) uart_putc('0');
    if (ms < 10)  uart_putc('0');
    kprintf("%d (Ticks: %d)\n", (int)ms, (int)ticks);
}
```

```c

    else if (mystrcmp(cmd, "uptime") == 0 || mystrcmp(cmd, "time") == 0) {
        print_uptime();
    }

```
