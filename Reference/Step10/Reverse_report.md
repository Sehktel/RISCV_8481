```bash
$ make all && make run
/usr/bin/docker run --rm --user $(id -u):$(id -g) -v $(pwd):/workspace -w /workspace riscv_os_devsecops:latest sh -c "rm -rf build bin/*"
/usr/bin/docker run --rm --user $(id -u):$(id -g) -v $(pwd):/workspace -w /workspace riscv_os_devsecops:latest sh -c "cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-toolchain.cmake . && cmake --build build"
-- The C compiler identification is GNU 13.2.0
-- The ASM compiler identification is GNU
-- Found assembler: /usr/bin/riscv64-unknown-elf-gcc
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/riscv64-unknown-elf-gcc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Configuring done (0.6s)
-- Generating done (0.0s)
-- Build files have been written to: /workspace/build
[ 33%] Building C object CMakeFiles/kernel.elf.dir/src/main.c.obj
[ 66%] Building ASM object CMakeFiles/kernel.elf.dir/src/start.S.obj
[100%] Linking C executable /workspace/bin/kernel.elf
Генерация сырого бинарника: bin/kernel.bin
[100%] Built target kernel.elf
qemu-system-riscv64 -M virt -bios none -kernel bin/kernel.bin -nographic

--- RISCV Микроядро: Shell загружен ---
sh> help
Доступные команды:
  help         - Вывод этого списка команд
  info         - Вывод системной информации ядра
  clear        - Очистить экран терминала
  time, uptime - Показать время работы системы
  mem          - Показать карту кучи (heap)
  memtest      - Выполнить тест аллокатора памяти
  usermode     - Запустить задачу в User Mode (U-mode + ecall)
  shed         - Проверка вытесняющей многозадачности
  pmptest      - Проверка аппаратной защита PMP  exit, quit   - Завершение работы программы
sh> mem
=== Карта кучи (Heap Map) ===
[0] Block: 0x800043e0 | Payload: 0x80004400 | Size: 132103168 B | FREE
Всего блоков: 1 | Занято: 0 B | Свободно: 132103168 B
sh> memtest
=== 1. Тест выделения и коалесценции ===
=== Карта кучи (Heap Map) ===
[0] Block: 0x800043e0 | Payload: 0x80004400 | Size: 64 B | USED
[1] Block: 0x80004440 | Payload: 0x80004460 | Size: 128 B | USED
[2] Block: 0x800044e0 | Payload: 0x80004500 | Size: 256 B | USED
[3] Block: 0x80004600 | Payload: 0x80004620 | Size: 132102624 B | FREE
Всего блоков: 4 | Занято: 448 B | Свободно: 132102624 B

--- Освобождаем средний блок (p2) ---
=== Карта кучи (Heap Map) ===
[0] Block: 0x800043e0 | Payload: 0x80004400 | Size: 64 B | USED
[1] Block: 0x80004440 | Payload: 0x80004460 | Size: 128 B | FREE
[2] Block: 0x800044e0 | Payload: 0x80004500 | Size: 256 B | USED
[3] Block: 0x80004600 | Payload: 0x80004620 | Size: 132102624 B | FREE
Всего блоков: 4 | Занято: 320 B | Свободно: 132102752 B

--- Освобождаем p1 (слияние вперёд с p2) ---
=== Карта кучи (Heap Map) ===
[0] Block: 0x800043e0 | Payload: 0x80004400 | Size: 224 B | FREE
[1] Block: 0x800044e0 | Payload: 0x80004500 | Size: 256 B | USED
[2] Block: 0x80004600 | Payload: 0x80004620 | Size: 132102624 B | FREE
Всего блоков: 3 | Занято: 256 B | Свободно: 132102848 B

--- Освобождаем p3 (слияние назад, куча должна стать единой) ---
=== Карта кучи (Heap Map) ===
[0] Block: 0x800043e0 | Payload: 0x80004400 | Size: 132103168 B | FREE
Всего блоков: 1 | Занято: 0 B | Свободно: 132103168 B

=== 2. Тест безопасности (Security Checks) ===
Попытка Double-Free на p3:
[SECURITY] kfree: Double-Free detected at 0x80004500!
Попытка освободить невыровненный указатель:
[SECURITY] kfree: unaligned pointer 0x80005003
Попытка освободить указатель вне кучи (например, из стека/кода):
[SECURITY] kfree: pointer out of heap bounds 0x80000010
sh> shed
[SECURITY] Use-after-free detected at 0x800043e0 (payload перезаписан после kfree)
[SCHED] Запуск вытесняющего планировщика Round-Robin (2 потока U-mode, квант 100 мс)...
  [TASK A | U-mode] Выполняю вычисления...
[TIMER IRQ #1] Переключение контекста (Round-Robin)...
  [TASK B | U-mode] Параллельный поток активен!
[TIMER IRQ #2] Переключение контекста (Round-Robin)...
[TIMER IRQ #3] Переключение контекста (Round-Robin)...
[TIMER IRQ #4] Переключение контекста (Round-Robin)...
[TIMER IRQ #5] Переключение контекста (Round-Robin)...
[TIMER IRQ #6] Переключение контекста (Round-Robin)...
[SCHED] Демонстрация планировщика завершена. Освобождаем память задач...
sh> usermode
[SECURITY] Use-after-free detected at 0x800043e0 (payload перезаписан после kfree)
[KERNEL] Опускаем процессор в U-mode (entry=0x800010c6, user_sp=0x80005400)...
[U-MODE] Привет из непривилегированного пространства! Запрашиваю тики...
[U-MODE] Системный вызов SYS_GET_TICKS вернул ненулевое время!
[U-MODE] Вызываю SYS_EXIT(0) для возврата в ядро...
[KERNEL] Процесс U-mode завершился через SYS_EXIT (код: 0)
[KERNEL] Управление успешно возвращено в M-mode Shell!
sh> pmptest
[SECURITY] Use-after-free detected at 0x800043e0 (payload перезаписан после kfree)
[KERNEL] Запускаем задачу-нарушитель U-mode со строгим PMP...
[U-MODE | ROGUE] Пытаюсь напрямую записать символ в UART0 (0x10000000)...

[SECURITY] PMP Violation! Store/AMO Access Fault (Запись в защищённую область / MMIO)
[SECURITY] mcause=7 | mepc=0x80001a58 | mtval=0x10000000 — процесс принудительно завершён!
[KERNEL] Ядро устояло, управление возвращено в M-mode Shell!
sh>
```
