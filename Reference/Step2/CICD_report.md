# Консоль 1

Используем shell-Makefile обертку, чтобы не ходить в build

```bash
sukhov@vm1545930:~/Work/RISCV_8481$ make clean
rm -rf build
rm -f bin/kernel.elf bin/kernel.bin


sukhov@vm1545930:~/Work/RISCV_8481$ make run
-- The C compiler identification is GNU 14.2.0
-- The ASM compiler identification is GNU
-- Found assembler: /usr/bin/riscv64-unknown-elf-gcc
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/riscv64-unknown-elf-gcc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Configuring done (0.3s)
-- Generating done (0.0s)
-- Build files have been written to: /home/sukhov/Work/RISCV_8481/build
gmake[1]: Entering directory '/home/sukhov/Work/RISCV_8481/build'
[ 33%] Building C object CMakeFiles/kernel.elf.dir/src/main.c.obj
[ 66%] Building ASM object CMakeFiles/kernel.elf.dir/src/start.S.obj
[100%] Linking C executable /home/sukhov/Work/RISCV_8481/bin/kernel.elf
/usr/lib/gcc/riscv64-unknown-elf/14.2.0/../../../riscv64-unknown-elf/bin/ld: warning: /home/sukhov/Work/RISCV_8481/bin/kernel.elf has a LOAD segment with RWX permissions
Генерация сырого бинарника: bin/kernel.bin
[100%] Built target kernel.elf
gmake[1]: Leaving directory '/home/sukhov/Work/RISCV_8481/build'
qemu-system-riscv64 -M virt -bios none -kernel bin/kernel.bin -nographic
Hello World!
sukhov@vm1545930:~/Work/RISCV_8481$
```

# Вывод

Проект переведен на CMake