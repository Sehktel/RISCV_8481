#ifndef KERNEL_LAYOUT_H
#define KERNEL_LAYOUT_H

/* Символы из src/linker.ld.
 * Объявлены как массивы без размера: в C это превращает идентификатор
 * в адрес, поэтому &_text_start — начало секции, а не чтение памяти по ней. */

extern char _text_start[];
extern char _text_end[];    /* ВАЖНО: стоит ПОСЛЕ .rodata — на это опирается pmp_configure_user (PMP TOR R+X регион) */
extern char _bss_start[];
extern char _bss_end[];
extern char _stack_bottom[];
extern char _stack_top[];
extern char _heap_start[];
extern char _heap_end[];
extern char _ram_end[];

#endif /* KERNEL_LAYOUT_H */
