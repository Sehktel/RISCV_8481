CC = riscv64-unknown-elf-gcc
OBJCOPY = riscv64-unknown-elf-objcopy
# GDB = riscv64-unknown-elf-gdb
GDB = gdb-multiarch

CFLAGS = -Wall -Wextra -g -O0 -ffreestanding -nostdlib -nostartfiles -mcmodel=medany

all: kernel.bin

kernel.elf: start.S main.c linker.ld
	$(CC) $(CFLAGS) -T linker.ld start.S main.c -o kernel.elf

kernel.bin: kernel.elf
	$(OBJCOPY) -O binary kernel.elf kernel.bin

run: kernel.bin
	qemu-system-riscv64 -M virt -bios none -kernel kernel.bin -nographic

# Запуск QEMU в режиме отладки:
# -S замораживает процессор на старте (до выполнения первой инструкции)
# -s открывает TCP-порт 1234 для подключения GDB
debug: kernel.bin
	qemu-system-riscv64 -M virt -bios none -kernel kernel.bin -nographic -s -S

# Быстрый запуск GDB с автоматическим подключением к QEMU
gdb: kernel.elf
	$(GDB) -ex "target remote localhost:1234" kernel.elf

clean:
	rm -f kernel.elf kernel.bin
