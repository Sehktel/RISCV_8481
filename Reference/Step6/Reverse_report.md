```bash
qemu-system-riscv64 -M virt -bios none -kernel bin/kernel.bin -nographic

--- RISCV Микроядро: Shell загружен ---
sh> time
Uptime: 00:02.763 (Ticks: 27631629)
sh> uptime
Uptime: 00:07.968 (Ticks: 79686161)
sh> exit
Завершение работы системы...
```