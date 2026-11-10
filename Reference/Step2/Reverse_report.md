# Консоль 1

```bash
$ make debug
gmake[1]: Entering directory '/home/sukhov/Work/RISCV_8481/build'
[100%] Built target kernel.elf
gmake[1]: Leaving directory '/home/sukhov/Work/RISCV_8481/build'
=========================================================
QEMU запущен в режиме отладки и ожидает GDB на порту 1234
Для подключения откройте новый терминал и выполните: make gdb
=========================================================
qemu-system-riscv64 -M virt -bios none -kernel bin/kernel.bin -nographic -s -S
Hello World!
```

# Консоль 2

```bash
$ make gdb
gdb-multiarch -ex "target remote localhost:1234" bin/kernel.elf
GNU gdb (Debian 16.3-1) 16.3
Copyright (C) 2024 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "x86_64-linux-gnu".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from bin/kernel.elf...
Remote debugging using localhost:1234
0x0000000000001000 in ?? ()
(gdb) list
34          while (1) {
35              __asm__ volatile("wfi");
36          }
37      }
38
39      int main(void) {
40          print("Hello World!\n");
41
42          // Автоматически тушим QEMU после завершения работы программы
43          poweroff();
(gdb) q
A debugging session is active.

        Inferior 1 [process 1] will be detached.

Quit anyway? (y or n) y
Detaching from program: /home/sukhov/Work/RISCV_8481/bin/kernel.elf, process 1
Ending remote debugging.
[Inferior 1 (process 1) detached]

```

# Вывод

Мы получили управляемую разработку.
Проект компилируется, запускается на QEMU
Останавливается и ждет команды удаленной отладки