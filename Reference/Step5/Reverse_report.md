```bash
$ make
docker run --rm \
        --user $(id -u):$(id -g) \
        -v $(pwd):/workspace \
        riscv_os_devsecops:latest \
        rm -rf build bin/*
docker run --rm \
        --user $(id -u):$(id -g) \
        -v $(pwd):/workspace \
        riscv_os_devsecops:latest \
        sh -c "cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-toolchain.cmake . && cmake --build build"
-- The C compiler identification is GNU 13.2.0
-- The ASM compiler identification is GNU
-- Found assembler: /usr/bin/riscv64-unknown-elf-gcc
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/riscv64-unknown-elf-gcc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Configuring done (1.1s)
-- Generating done (0.0s)
-- Build files have been written to: /workspace/build
[ 33%] Building C object CMakeFiles/kernel.elf.dir/src/main.c.obj
[ 66%] Building ASM object CMakeFiles/kernel.elf.dir/src/start.S.obj
[100%] Linking C executable /workspace/bin/kernel.elf
/usr/lib/gcc/riscv64-unknown-elf/13.2.0/../../../riscv64-unknown-elf/bin/ld: warning: /workspace/bin/kernel.elf has a LOAD segment with RWX permissions
Генерация сырого бинарника: bin/kernel.bin
[100%] Built target kernel.elf
qemu-system-riscv64 -M virt -bios none -kernel bin/kernel.bin -nographic

--- RISCV_8481 Микроядро: Shell загружен ---
sh> gelp
Ошибка: неизвестная команда 'рудз'. Введите 'help'
sh> help
Доступные команды:
  help  - Вывод этого списка команд
  info  - Вывод системной информации ядра
  clear - Очистить экран терминала
  exit  - Завершение работы программы
sh> info
--- Диагностика ядра RISCV_8481 ---
Архитектура: RV64GC (64-bit)
Базовый адрес UART0: 0x10000000
Текущий указатель стека (SP): 0x80001910
sh> exit
Завершение работы системы...
```


```bash
$ .git/hooks/pre-commit
=========================================================
    DevSecOps: Запуск pre-commit проверок безопасности...
=========================================================
Очистка trailing-пробелов в измененных файлах...
Файлы очищены от мусорных пробелов.
Запуск SAST-анализа кода через Cppcheck...
Checking src/main.c ...
Все проверки успешно пройдены! Коммит разрешен.
```