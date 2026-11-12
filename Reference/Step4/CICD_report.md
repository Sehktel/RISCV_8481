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
-- Configuring done (0.4s)
-- Generating done (0.0s)
-- Build files have been written to: /workspace/build
[ 33%] Building C object CMakeFiles/kernel.elf.dir/src/main.c.obj
/workspace/src/main.c: In function 'execute_command':
/workspace/src/main.c:110:9: warning: variable 'test_array' set but not used [-Wunused-but-set-variable]
  110 |     int test_array[5];
      |         ^~~~~~~~~~
[ 66%] Building ASM object CMakeFiles/kernel.elf.dir/src/start.S.obj
[100%] Linking C executable /workspace/bin/kernel.elf
/usr/lib/gcc/riscv64-unknown-elf/13.2.0/../../../riscv64-unknown-elf/bin/ld: warning: /workspace/bin/kernel.elf has a LOAD segment with RWX permissions
Генерация сырого бинарника: bin/kernel.bin
[100%] Built target kernel.elf
qemu-system-riscv64 -M virt -bios none -kernel bin/kernel.bin -nographic

--- RISCV Микроядро: Shell загружен ---
sh> exit
Завершение работы программы...
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
src/main.c:111:15: error: Array 'test_array[5]' accessed at index 10, which is out of bounds. [arrayIndexOutOfBounds]
    test_array[10] = 42;
              ^
Критическая ошибка: Cppcheck нашел баги или уязвимости в коде!
Коммит заблокирован. Исправьте предупреждения выше.
```