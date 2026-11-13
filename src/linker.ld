OUTPUT_ARCH( "riscv" )
ENTRY( _start )

MEMORY
{
  ram (rwx) : ORIGIN = 0x80000000, LENGTH = 128M
}

/* Явные программные заголовки: линкер больше не склеивает всё в один RWX-сегмент */
PHDRS
{
  text PT_LOAD FLAGS(5);  /* R-X: код и константы */
  data PT_LOAD FLAGS(6);  /* RW-: данные, bss, куча */
}

SECTIONS
{
  . = ORIGIN(ram);

  .text : {
    *(.text.init)
    *(.text .text.*)
  } > ram :text

  /* .rodata тоже можно держать в text-сегменте: он R-X, а константам
     исполнение не мешает. Хотите строже — вынесите в отдельный
     PT_LOAD FLAGS(4) (только R). */
  .rodata : {
    *(.rodata .rodata.*)
  } > ram :text

  .data : {
    *(.data .data.*)
  } > ram :data

  .bss (NOLOAD) : {
    . = ALIGN(8);
    _bss_start = .;
    *(.bss .bss.*)
    *(COMMON)
    . = ALIGN(8);
    _bss_end = .;
  } > ram :data

  /* Стек — отдельная NOLOAD-секция, а не зазор в счётчике адресов */
  .stack (NOLOAD) : ALIGN(16) {
    _stack_bottom = .;
    . += 0x1000;          /* 4 КБ под стек */
    _stack_top = .;
  } > ram :data

  /* Куча: от вершины стека до конца RAM */
  .heap (NOLOAD) : ALIGN(16) {
    _heap_start = .;
    . += ORIGIN(ram) + LENGTH(ram) - . - 0x200000; /* оставляем 2 МБ запаса */
    _heap_end = .;
  } > ram :data
}
