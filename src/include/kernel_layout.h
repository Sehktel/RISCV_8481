#ifndef KERNEL_LAYOUT_H
#define KERNEL_LAYOUT_H

/* Символы из src/linker.ld.
 * Объявлены как массивы без размера: в C это превращает идентификатор
 * в адрес, поэтому &kernel_text_start — начало секции, а не чтение памяти по ней. */

extern char kernel_text_start[];
extern char kernel_text_end[];

extern char kernel_bss_start[];
extern char kernel_bss_end[];

extern char kernel_stack_bottom[];
extern char kernel_stack_top[];

extern char kernel_heap_start[];
extern char kernel_heap_end[];

extern char kernel_ram_end[];

extern char kernel_pgtables_start[];
extern char kernel_pgtables_end[];

#endif /* KERNEL_LAYOUT_H */
