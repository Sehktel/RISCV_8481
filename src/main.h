// Регистры управления питанием QEMU (SiFive Test Finisher)
#define SYSCON_TEST_ADDR 0x100000
#define FINISHER_PASS 0x5555

// Адрес UART0 на плате QEMU virt
#define UART0_BASE 0x10000000UL

/// Регистр данных UART (запись — transmit, чтение — receive)
#define UART0_THR ((volatile unsigned char*)(UART0_BASE + 0x00))
/// Регистр статуса линии (бит 0 — Data Ready, бит 5 — THRE)
#define UART0_LSR ((volatile unsigned char*)(UART0_BASE + 0x05))

// Константы битов прерываний RISC-V
#define MSTATUS_MIE (1ULL << 3) // Глобальное разрешение прерываний в Machine-mode
#define MIE_MTIE (1ULL << 7)    // Разрешение прерываний от машинного таймера

/* ============================================================================
 * БАЗОВЫЕ ТИПЫ (RV64 / LP64 Bare-Metal)
 * ============================================================================ */
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long int64_t;
typedef unsigned long uint64_t;

typedef unsigned long size_t;
typedef signed long ssize_t;
typedef unsigned long uintptr_t;
typedef signed long intptr_t;

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef _Bool bool;
#define true 1
#define false 0

#define clear_csr(reg, bit) ({ asm volatile("csrc " #reg ", %0" ::"r"(bit)); })
#define write_csr(reg, val) ({ asm volatile("csrw " #reg ", %0" ::"r"(val)); })
#define set_csr(reg, bit) ({ asm volatile("csrs " #reg ", %0" ::"r"(bit)); })

#define BUFFER_SIZE 64

/// Частота таймера: 10 МГц для QEMU virt, 25 МГц для реального железа
#define TIMER_FREQ 10000000ULL

extern char _heap_start[];
extern char _heap_end[];

#define ALIGN16(x) (((x) + 15) & ~((size_t)15))

// Магические константы для контроля целостности заголовка (64-бит)
#define HEAP_MAGIC_ALLOC 0xDEADC0DE12345678ULL
#define HEAP_MAGIC_FREE 0xFEE1DEAD87654321ULL

typedef struct block_header {
    uint64_t magic;            // 8 байт: страж целостности и статуса
    size_t size;               // 8 байт: размер полезной нагрузки
    struct block_header* next; // 8 байт: следующий блок
    uint8_t is_free;           // 1 байт (+ 7 байт паддинга)
} __attribute__((aligned(16))) block_header_t;

// Гарантируем на этапе компиляции, что размер заголовка кратен 16 (32 байта)
_Static_assert(sizeof(block_header_t) % 16 == 0, "block_header_t must be 16-byte aligned");

static block_header_t* heap_head = NULL;
static uintptr_t heap_bounds_start = 0;
static uintptr_t heap_bounds_end = 0;

// Яд для детекции use-after-free: свободная память обязана содержать этот байт
#define HEAP_POISON_BYTE 0xA5
// Сколько байтов payload проверяем на яд при каждом выделении
#define HEAP_UAF_CHECK_BYTES 16

void heap_init(void);
void heap_dump(void);
void heap_test(void);
void* kmalloc(size_t size);
void kfree(void* ptr);
void enable_interrupts(void);
char uart_getc(void);
int mystrcmp(const char* s1, const char* s2);
int mystrlen(const char* s);
void uart_putc(char c);
void print(const char* str);
void poweroff(void);
static void print_number(uint64_t num, int base, int min_width);
void kprintf(const char* fmt, ...);
uint64_t get_ticks(void);
void print_uptime(void);
void execute_command(const char* cmd);
