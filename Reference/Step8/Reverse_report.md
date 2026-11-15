```bash
$ make format
docker run --rm \
        --user $(id -u):$(id -g) \
        -v $(pwd):/workspace \
        riscv_os_devsecops:latest \
        sh -c "find src/ -name '*.c' -o -name '*.h' | xargs clang-format -i"
Код в папке src/ успешно отформатирован по стандарту проекта.

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
-- Configuring done (0.3s)
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
sh> usermode
[KERNEL] Опускаем процессор в U-mode (entry=0x80001058, user_sp=0x80004410)...
[U-MODE] Привет из непривилегированного пространства! Запрашиваю тики...
[U-MODE] Системный вызов SYS_GET_TICKS вернул ненулевое время!
[U-MODE] Вызываю SYS_EXIT(0) для возврата в ядро...
[KERNEL] Процесс U-mode завершился через SYS_EXIT (код: 0)
[KERNEL] Управление успешно возвращено в M-mode Shell!
sh> quit
Завершение работы системы...
```