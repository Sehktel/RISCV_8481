```bash
$ make clear
rm -rf build bin/*

$ make all
docker run --rm --user $(id -u):$(id -g) -v $(pwd):/workspace riscv_os_devsecops:latest make clean
rm -rf build bin/*
docker run --rm --user $(id -u):$(id -g) -v $(pwd):/workspace riscv_os_devsecops:latest make build
cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-toolchain.cmake .
-- The C compiler identification is GNU 13.2.0
-- The ASM compiler identification is GNU
-- Found assembler: /usr/bin/riscv64-unknown-elf-gcc
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/riscv64-unknown-elf-gcc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Configuring done (0.5s)
-- Generating done (0.0s)
-- Build files have been written to: /workspace/build
cmake --build build
gmake[1]: Entering directory '/workspace/build'
gmake[2]: Entering directory '/workspace/build'
gmake[3]: Entering directory '/workspace/build'
gmake[3]: Leaving directory '/workspace/build'
gmake[3]: Entering directory '/workspace/build'
[ 33%] Building C object CMakeFiles/kernel.elf.dir/src/main.c.obj
[ 66%] Building ASM object CMakeFiles/kernel.elf.dir/src/start.S.obj
[100%] Linking C executable /workspace/bin/kernel.elf
Генерация сырого бинарника: bin/kernel.bin
gmake[3]: Leaving directory '/workspace/build'
[100%] Built target kernel.elf
gmake[2]: Leaving directory '/workspace/build'
gmake[1]: Leaving directory '/workspace/build'
qemu-system-riscv64 -M virt -bios none -kernel bin/kernel.bin -nographic

--- RISCV Микроядро: Shell загружен ---
sh> mem
=== Карта кучи (Heap Map) ===
[0] Block: 0x80002c20 | Payload: 0x80002c40 | Size: 132109248 B | FREE
Всего блоков: 1 | Занято: 0 B | Свободно: 132109248 B
sh> help
Доступные команды:
  help         - Вывод этого списка команд
  info         - Вывод системной информации ядра
  clear        - Очистить экран терминала
  time, uptime - Показать время работы системы
  mem          - Показать карту кучи (heap)
  memtest      - Выполнить тест аллокатора памяти
  exit, quit   - Завершение работы программы
sh> memtest
=== 1. Тест выделения и коалесценции ===
=== Карта кучи (Heap Map) ===
[0] Block: 0x80002c20 | Payload: 0x80002c40 | Size: 64 B | USED
[1] Block: 0x80002c80 | Payload: 0x80002ca0 | Size: 128 B | USED
[2] Block: 0x80002d20 | Payload: 0x80002d40 | Size: 256 B | USED
[3] Block: 0x80002e40 | Payload: 0x80002e60 | Size: 132108704 B | FREE
Всего блоков: 4 | Занято: 448 B | Свободно: 132108704 B

--- Освобождаем средний блок (p2) ---
=== Карта кучи (Heap Map) ===
[0] Block: 0x80002c20 | Payload: 0x80002c40 | Size: 64 B | USED
[1] Block: 0x80002c80 | Payload: 0x80002ca0 | Size: 128 B | FREE
[2] Block: 0x80002d20 | Payload: 0x80002d40 | Size: 256 B | USED
[3] Block: 0x80002e40 | Payload: 0x80002e60 | Size: 132108704 B | FREE
Всего блоков: 4 | Занято: 320 B | Свободно: 132108832 B

--- Освобождаем p1 (слияние вперёд с p2) ---
=== Карта кучи (Heap Map) ===
[0] Block: 0x80002c20 | Payload: 0x80002c40 | Size: 224 B | FREE
[1] Block: 0x80002d20 | Payload: 0x80002d40 | Size: 256 B | USED
[2] Block: 0x80002e40 | Payload: 0x80002e60 | Size: 132108704 B | FREE
Всего блоков: 3 | Занято: 256 B | Свободно: 132108928 B

--- Освобождаем p3 (слияние назад, куча должна стать единой) ---
=== Карта кучи (Heap Map) ===
[0] Block: 0x80002c20 | Payload: 0x80002c40 | Size: 132109248 B | FREE
Всего блоков: 1 | Занято: 0 B | Свободно: 132109248 B

=== 2. Тест безопасности (Security Checks) ===
Попытка Double-Free на p3:
[SECURITY] kfree: Double-Free detected at 0x80002d40!
Попытка освободить невыровненный указатель:
[SECURITY] kfree: unaligned pointer 0x80005003
Попытка освободить указатель вне кучи (например, из стека/кода):
[SECURITY] kfree: pointer out of heap bounds 0x80000010
sh> exit
Завершение работы системы...
```

```
2026-11-15T09:35:11.3048397Z riscv-local-runner(version:v5.0.2) received task 3 of job 3, be triggered by event: push
2026-11-15T09:35:11.3051405Z workflow prepared
2026-11-15T09:35:11.3053017Z 🚀  Start image=riscv_os_devsecops:latest
2026-11-15T09:35:11.3137275Z   🐳  docker pull image=riscv_os_devsecops:latest platform= username= forcePull=false
2026-11-15T09:35:11.3186087Z   🐳  docker create image=riscv_os_devsecops:latest platform= entrypoint=["tail" "-f" "/dev/null"] cmd=[] network="devsecops_net"
2026-11-15T09:35:11.3502834Z   🐳  docker run image=riscv_os_devsecops:latest platform= entrypoint=["tail" "-f" "/dev/null"] cmd=[] network="devsecops_net"
2026-11-15T09:35:11.7635754Z ⭐ Run Main Checkout repository
2026-11-15T09:35:11.8085036Z   🐳  docker exec cmd=[sh -e /var/run/act/workflow/0.sh] user= workdir=
2026-11-15T09:35:11.9053435Z Cloning into '.'...
2026-11-15T09:35:12.2524957Z find src/ -name '*.c' -o -name '*.h' | xargs clang-format --dry-run --Werror
2026-11-15T09:35:12.5464740Z cppcheck --enable=warning,style,performance,portability \
2026-11-15T09:35:12.5466469Z          --platform=unix64 \
2026-11-15T09:35:12.5467112Z          --library=.infrastructure/cppcheck/kernel.cfg \
2026-11-15T09:35:12.5467396Z          --error-exitcode=1 \
2026-11-15T09:35:12.5467511Z          --suppress=missingIncludeSystem \
2026-11-15T09:35:12.5467592Z          --suppress=unusedFunction \
2026-11-15T09:35:12.5467672Z          --inline-suppr \
2026-11-15T09:35:12.5467763Z          src/
2026-11-15T09:35:12.5548525Z Checking src/main.c ...
2026-11-15T09:35:12.8076704Z cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/riscv64-toolchain.cmake .
2026-11-15T09:35:13.0356694Z -- The C compiler identification is GNU 13.2.0
2026-11-15T09:35:13.0505353Z -- The ASM compiler identification is GNU
2026-11-15T09:35:13.0513675Z -- Found assembler: /usr/bin/riscv64-unknown-elf-gcc
2026-11-15T09:35:13.0690264Z -- Detecting C compiler ABI info
2026-11-15T09:35:13.2613410Z -- Detecting C compiler ABI info - done
2026-11-15T09:35:13.2657784Z -- Check for working C compiler: /usr/bin/riscv64-unknown-elf-gcc - skipped
2026-11-15T09:35:13.2665493Z -- Detecting C compile features
2026-11-15T09:35:13.2673718Z -- Detecting C compile features - done
2026-11-15T09:35:13.2760679Z -- Configuring done (0.4s)
2026-11-15T09:35:13.2794016Z -- Generating done (0.0s)
2026-11-15T09:35:13.2800429Z -- Build files have been written to: /workspace/SukhovAA/RISCV/build
2026-11-15T09:35:13.2840318Z cmake --build build
2026-11-15T09:35:13.2961901Z gmake[1]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-15T09:35:13.3192888Z gmake[2]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-15T09:35:13.3205548Z gmake[3]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-15T09:35:13.3329391Z gmake[3]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-15T09:35:13.3348464Z gmake[3]: Entering directory '/workspace/SukhovAA/RISCV/build'
2026-11-15T09:35:13.3457991Z [ 33%] Building C object CMakeFiles/kernel.elf.dir/src/main.c.obj
2026-11-15T09:35:13.4071151Z [ 66%] Building ASM object CMakeFiles/kernel.elf.dir/src/start.S.obj
2026-11-15T09:35:13.4304751Z [100%] Linking C executable /workspace/SukhovAA/RISCV/bin/kernel.elf
2026-11-15T09:35:13.4654484Z Генерация сырого бинарника: bin/kernel.bin
2026-11-15T09:35:13.4748516Z gmake[3]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-15T09:35:13.4868359Z [100%] Built target kernel.elf
2026-11-15T09:35:13.4872162Z gmake[2]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-15T09:35:13.4986348Z gmake[1]: Leaving directory '/workspace/SukhovAA/RISCV/build'
2026-11-15T09:35:13.5044206Z -rwxr-xr-x 1 root root 7.1K Nov 15 09:35 bin/kernel.bin
2026-11-15T09:35:13.5045119Z -rwxr-xr-x 1 root root  24K Nov 15 09:35 bin/kernel.elf
2026-11-15T09:35:13.5200624Z Cleaning up container for job build-and-sast
2026-11-15T09:35:13.6751771Z 🏁  Job succeeded
```