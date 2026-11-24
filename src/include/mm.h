#ifndef MM_H
#define MM_H

#include "./include/types.h"

// Магические константы для контроля целостности заголовка (64-бит)
#define HEAP_MAGIC_ALLOC 0xDEADC0DE12345678ULL
#define HEAP_MAGIC_FREE 0xFEE1DEAD87654321ULL

// Яд для детекции use-after-free: свободная память обязана содержать этот байт
#define HEAP_POISON_BYTE 0xA5
// Сколько байтов payload проверяем на яд при каждом выделении
#define HEAP_UAF_CHECK_BYTES 16

// NOLINTBEGIN(bugprone-reserved-identifier)
extern char _heap_start[];
extern char _heap_end[];
// NOLINTEND(bugprone-reserved-identifier)

typedef struct block_header {
    uint64_t magic;            // 8 байт: страж целостности и статуса
    size_t size;               // 8 байт: размер полезной нагрузки
    struct block_header* next; // 8 байт: следующий блок
    uint8_t is_free;           // 1 байт (+ 7 байт паддинга)
} __attribute__((aligned(16))) block_header_t;

void heap_init(void);
void heap_dump(void);
void heap_test(void);
void* kmalloc(size_t size);
// void kfree(void* ptr);

// Гарантируем на этапе компиляции, что размер заголовка кратен 16 (32 байта)
_Static_assert(sizeof(block_header_t) % 16 == 0, "block_header_t must be 16-byte aligned");

#endif /* MM_H */
