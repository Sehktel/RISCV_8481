CC = riscv64-unknown-elf-gcc
SRC = src/
BIN = bin/
OBJCOPY = riscv64-unknown-elf-objcopy
# GDB = riscv64-unknown-elf-gdb
GDB = gdb-multiarch

CFLAGS = -Wall -Wextra -g -O0 -ffreestanding -nostdlib -nostartfiles -mcmodel=medany
KERNEL = $(BIN)kernel.bin

all: $(KERNEL)

kernel.elf: $(SRC)start.S $(SRC)main.c $(SRC)linker.ld
	$(CC) $(CFLAGS) -T $(SRC)linker.ld $(SRC)start.S $(SRC)main.c -o $(BIN)kernel.elf

$(KERNEL): kernel.elf
	$(OBJCOPY) -O binary $(BIN)kernel.elf $@

run: $(KERNEL)
	qemu-system-riscv64 -M virt -bios none -kernel $< -nographic

# Запуск QEMU в режиме отладки:
# -S замораживает процессор на старте (до выполнения первой инструкции)
# -s открывает TCP-порт 1234 для подключения GDB
debug: $(KERNEL)
	qemu-system-riscv64 -M virt -bios none -kernel $< -nographic -s -S

# Быстрый запуск GDB с автоматическим подключением к QEMU
gdb: kernel.elf
	$(GDB) -ex "target remote localhost:1234" bin/kernel.elf

clean:
	rm -f bin/kernel.elf $(KERNEL)
