# main.c

```c
#define UART0_BASE 0x10000000
#define UART0_THR  ((volatile unsigned char*)(UART0_BASE + 0))
#define UART0_LSR  ((volatile unsigned char*)(UART0_BASE + 5))

void uart_putc(char c) {
    while ((*UART0_LSR & (1 << 5)) == 0);
    *UART0_THR = c;
}

void uart_puts(const char *s) {
    while (*s) {
        uart_putc(*s++);
    }
}

int main(void) {
    uart_puts("Hello World!\n");
    while (1) {
        __asm__ volatile("wfi");
    }
    return 0;
}
```

# linker.ld
```
OUTPUT_ARCH( "riscv" )
ENTRY( _start )

MEMORY
{
  ram (rwx) : ORIGIN = 0x80000000, LENGTH = 128M
}

SECTIONS
{
  . = ORIGIN(ram);

  .text : {
    *(.text.init)
    *(.text .text.*)
  } > ram

  .rodata : {
    *(.rodata .rodata.*)
  } > ram

  .data : {
    *(.data .data.*)
  } > ram

  .bss : {
    *(.bss .bss.*)
  } > ram

  /* Выделяем 4КБ под стек */
  . = ALIGN(16);
  . += 0x1000;
  _stack_top = .;
}
```

# start.S

```asm
.section .text.init
.global _start

_start:
    # 1. Отключаем все ядра, кроме основного (hart 0)
    csrr a0, mhartid            # Прочитать ID текущего ядра
    bnez a0, park               # Если это не ядро 0, отправляем в бесконечный цикл

    # 2. Настраиваем указатель стека (SP)
    la sp, _stack_top

    # 3. Переходим к коду на C
    j main

park:
    wfi                         # Ожидание прерывания (сон)
    j park
```