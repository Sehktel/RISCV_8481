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
-- Configuring done (0.3s)
-- Generating done (0.0s)
-- Build files have been written to: /workspace/build
[ 33%] Building C object CMakeFiles/kernel.elf.dir/src/main.c.obj
[ 66%] Building ASM object CMakeFiles/kernel.elf.dir/src/start.S.obj
[100%] Linking C executable /workspace/bin/kernel.elf
Генерация сырого бинарника: bin/kernel.bin
[100%] Built target kernel.elf
qemu-system-riscv64 -M virt -bios none -kernel bin/kernel.bin -nographic

--- RISCV Микроядро: Shell загружен ---
sh> mem
=== Карта кучи (Heap Map) ===
[0] Block: 0x80002bf0 | Payload: 0x80002c10 | Size: 132109296 B | FREE
Всего блоков: 1 | Занято: 0 B | Свободно: 132109296 B
sh> memtest
=== 1. Тест выделения и коалесценции ===
=== Карта кучи (Heap Map) ===
[0] Block: 0x80002bf0 | Payload: 0x80002c10 | Size: 64 B | USED
[1] Block: 0x80002c50 | Payload: 0x80002c70 | Size: 128 B | USED
[2] Block: 0x80002cf0 | Payload: 0x80002d10 | Size: 256 B | USED
[3] Block: 0x80002e10 | Payload: 0x80002e30 | Size: 132108752 B | FREE
Всего блоков: 4 | Занято: 448 B | Свободно: 132108752 B

--- Освобождаем средний блок (p2) ---
=== Карта кучи (Heap Map) ===
[0] Block: 0x80002bf0 | Payload: 0x80002c10 | Size: 64 B | USED
[1] Block: 0x80002c50 | Payload: 0x80002c70 | Size: 128 B | FREE
[2] Block: 0x80002cf0 | Payload: 0x80002d10 | Size: 256 B | USED
[3] Block: 0x80002e10 | Payload: 0x80002e30 | Size: 132108752 B | FREE
Всего блоков: 4 | Занято: 320 B | Свободно: 132108880 B

--- Освобождаем p1 (слияние вперёд с p2) ---
=== Карта кучи (Heap Map) ===
[0] Block: 0x80002bf0 | Payload: 0x80002c10 | Size: 224 B | FREE
[1] Block: 0x80002cf0 | Payload: 0x80002d10 | Size: 256 B | USED
[2] Block: 0x80002e10 | Payload: 0x80002e30 | Size: 132108752 B | FREE
Всего блоков: 3 | Занято: 256 B | Свободно: 132108976 B

--- Освобождаем p3 (слияние назад, куча должна стать единой) ---
=== Карта кучи (Heap Map) ===
[0] Block: 0x80002bf0 | Payload: 0x80002c10 | Size: 132109296 B | FREE
Всего блоков: 1 | Занято: 0 B | Свободно: 132109296 B

=== 2. Тест безопасности (Security Checks) ===
Попытка Double-Free на p3:
[SECURITY] kfree: Double-Free detected at 0x80002d10!
Попытка освободить невыровненный указатель:
[SECURITY] kfree: unaligned pointer 0x80005003
Попытка освободить указатель вне кучи (например, из стека/кода):
[SECURITY] kfree: pointer out of heap bounds 0x80000010
sh>