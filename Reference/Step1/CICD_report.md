# Запуск

```bash
make -f Makefile run
riscv64-unknown-elf-gcc -Wall -Wextra -g -O0 -ffreestanding -nostdlib -nostartfiles -mcmodel=medany -T src/linker.ld src/start.S src/main.c -o bin/kernel.elf
/usr/lib/gcc/riscv64-unknown-elf/14.2.0/../../../riscv64-unknown-elf/bin/ld: warning: bin/kernel.elf has a LOAD segment with RWX permissions
riscv64-unknown-elf-objcopy -O binary bin/kernel.elf bin/kernel.bin
qemu-system-riscv64 -M virt -bios none -kernel bin/kernel.bin -nographic
Hello World!
```

# Вывод

Базовый Hello World