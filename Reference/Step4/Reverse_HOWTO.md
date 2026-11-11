```c

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

```

```c
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
```

```c
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

```