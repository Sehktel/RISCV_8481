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
