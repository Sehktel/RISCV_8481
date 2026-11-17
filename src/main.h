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

// NOLINTBEGIN(bugprone-reserved-identifier)
extern char _text_start[];
extern char _text_end[];
extern char _heap_start[];
extern char _heap_end[];
// NOLINTEND(bugprone-reserved-identifier)

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

// Константы битов прерываний и привилегий RISC-V
#define MSTATUS_MIE (1ULL << 3)       // Глобальное разрешение прерываний в Machine-mode
#define MSTATUS_MPIE (1ULL << 7)      // Предыдущее состояние MIE до входа в ловушку
#define MSTATUS_MPP_MASK (3ULL << 11) // Маска режима привилегий (00=U, 01=S, 11=M)
#define MIE_MTIE (1ULL << 7) // Разрешение прерываний от машинного таймера

// Коды исключений (mcause при старшем бите = 0)
#define CAUSE_ILLEGAL_INSTRUCTION 2ULL
#define CAUSE_ECALL_FROM_UMODE 8ULL
#define CAUSE_ECALL_FROM_MMODE 11ULL

// Таблица системных вызовов (ABI RISC-V: номер в a7, аргументы в a0..a2)
#define SYS_PRINT 1ULL
#define SYS_GET_TICKS 2ULL
#define SYS_EXIT 93ULL

// Регистры аппаратного таймера CLINT на плате QEMU virt (Hart 0)
#define CLINT_BASE 0x02000000UL
#define CLINT_MTIMECMP0 ((volatile uint64_t*)(CLINT_BASE + 0x4000))
#define CLINT_MTIME ((volatile uint64_t*)(CLINT_BASE + 0xBFF8))

// Квант времени планировщика: 100 мс при частоте таймера 10 МГц (1 000 000 тиков)
#define SCHED_QUANTUM_TICKS (TIMER_FREQ / 10)

// Код прерывания машинного таймера (старший бит 63 = 1, код = 7)
#define MCAUSE_TIMER_INTERRUPT ((1ULL << 63) | 7ULL)

// Состояния потока и дескриптор задачи (Task Control Block)
#define MAX_TASKS 4

typedef enum { TASK_UNUSED = 0, TASK_READY = 1, TASK_RUNNING = 2, TASK_EXITED = 3 } task_state_t;

// Структура сохранённого контекста (должна строго соответствовать смещениям в trap_vector)
typedef struct {
    uint64_t ra;
    uint64_t gp;
    uint64_t tp;
    uint64_t t0;
    uint64_t t1;
    uint64_t t2;
    uint64_t s0;
    uint64_t s1;
    uint64_t a0;
    uint64_t a1;
    uint64_t a2;
    uint64_t a3;
    uint64_t a4;
    uint64_t a5;
    uint64_t a6;
    uint64_t a7;
    uint64_t s2;
    uint64_t s3;
    uint64_t s4;
    uint64_t s5;
    uint64_t s6;
    uint64_t s7;
    uint64_t s8;
    uint64_t s9;
    uint64_t s10;
    uint64_t s11;
    uint64_t t3;
    uint64_t t4;
    uint64_t t5;
    uint64_t t6;
    uint64_t sp;
    uint64_t padding;
} trap_frame_t;

_Static_assert(sizeof(trap_frame_t) == 256, "trap_frame_t must match 256-byte assembly frame");

typedef struct {
    int id;
    task_state_t state;
    uint64_t mepc;   // Сохранённый счётчик команд потока
    uint8_t* kstack; // Выделенный в куче стек ядра (на вершине лежит trap_frame_t)
    uint8_t* ustack; // Выделенный в куче стек пользователя
    trap_frame_t* frame; // Указатель на контекст 32 регистров в kstack
} task_t;

// Макрос чтения CSR
#define read_csr(reg)                                                                                                  \
    ({                                                                                                                 \
        uint64_t __tmp;                                                                                                \
        asm volatile("csrr %0, " #reg : "=r"(__tmp));                                                                  \
        __tmp;                                                                                                         \
    })

// Коды исключений (mcause при старшем бите = 0)
#define CAUSE_INSTRUCTION_ACCESS_FAULT 1ULL
#define CAUSE_ILLEGAL_INSTRUCTION 2ULL
#define CAUSE_LOAD_ACCESS_FAULT 5ULL
#define CAUSE_STORE_ACCESS_FAULT 7ULL
#define CAUSE_ECALL_FROM_UMODE 8ULL
#define CAUSE_ECALL_FROM_MMODE 11ULL

// Флаги конфигурации PMP (биты: R=0, W=1, X=2, A[4:3]=01 для TOR)
#define PMP_R (1ULL << 0)
#define PMP_W (1ULL << 1)
#define PMP_X (1ULL << 2)
#define PMP_TOR (1ULL << 3)

static task_t tasks[MAX_TASKS];
static int current_task_idx = -1;
static int scheduler_active = 0;
static int timer_ticks_count = 0;
static uintptr_t shell_return_sp = 0;

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

// void trap_handler(trap_frame_t* frame);
trap_frame_t* trap_handler(trap_frame_t* frame);

void run_scheduler_demo(void);
void enter_user_mode(uintptr_t entry, uintptr_t user_sp);
void exit_user_mode_return(void);
void run_user_mode_demo(void);

void pmp_configure_user(uintptr_t ustack_base, uintptr_t ustack_top);
void run_pmp_violation_demo(void);
