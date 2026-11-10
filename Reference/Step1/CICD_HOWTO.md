# Makefile

```make
CC = riscv64-unknown-elf-gcc
OBJCOPY = riscv64-unknown-elf-objcopy
CFLAGS = -Wall -Wextra -O2 -ffreestanding -nostdlib -nostartfiles -mcmodel=medany

all: kernel.bin

kernel.elf: start.S main.c linker.ld
	$(CC) $(CFLAGS) -T linker.ld start.S main.c -o kernel.elf

kernel.bin: kernel.elf
	$(OBJCOPY) -O binary kernel.elf kernel.bin

run: kernel.bin
	qemu-system-riscv64 -M virt -bios none -kernel kernel.bin -nographic

clean:
	rm -f kernel.elf kernel.bin
```
