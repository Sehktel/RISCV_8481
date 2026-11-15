start.S
```asm
.section .text.init
.global _start
.global trap_vector
.global enter_user_mode

_start:
    # 1. Отключаем все ядра, кроме основного (hart 0)
    csrr a0, mhartid            # Прочитать ID текущего ядра
    bnez a0, park               # Если это не ядро 0, отправляем в бесконечный цикл

    # 2. Настраиваем указатель стека (SP) и сохраняем вершину стека ядра в mscratch
    la sp, _stack_top
    csrw mscratch, sp

    # 3. Регистрируем обработчик ловушек в mtvec (режим Direct, младшие 2 бита = 0)
    la t0, trap_vector
    csrw mtvec, t0

    # 4. Переходим к коду на C с сохранением адреса возврата
    jal ra, kmain

    # 5. Ловушка на случай, если kmain всё-таки завершился
tail_loop:
    wfi
    j tail_loop

park:
    wfi
    j park

# ============================================================================
# ВЕКТОР ЛОВУШЕК (TRAP VECTOR) — выровнен по 4 байтам
# ============================================================================
.align 4
trap_vector:
    # Атомарно меняем местами sp (пользовательский) и mscratch (стек ядра)
    csrrw sp, mscratch, sp

    # Выделяем 256 байт (32 регистра * 8 байт) на стеке ядра для сохранения контекста
    addi sp, sp, -256

    sd ra,   0(sp)
    sd gp,   8(sp)
    sd tp,   16(sp)
    sd t0,   24(sp)
    sd t1,   32(sp)
    sd t2,   40(sp)
    sd s0,   48(sp)
    sd s1,   56(sp)
    sd a0,   64(sp)
    sd a1,   72(sp)
    sd a2,   80(sp)
    sd a3,   88(sp)
    sd a4,   96(sp)
    sd a5,   104(sp)
    sd a6,   112(sp)
    sd a7,   120(sp)
    sd s2,   128(sp)
    sd s3,   136(sp)
    sd s4,   144(sp)
    sd s5,   152(sp)
    sd s6,   160(sp)
    sd s7,   168(sp)
    sd s8,   176(sp)
    sd s9,   184(sp)
    sd s10,  192(sp)
    sd s11,  200(sp)
    sd t3,   208(sp)
    sd t4,   216(sp)
    sd t5,   224(sp)
    sd t6,   232(sp)

    # Сохраняем исходный пользовательский sp (сейчас лежит в mscratch) во фрейм
    csrr t0, mscratch
    sd t0,   240(sp)

    # Передаём указатель на trap_frame_t в a0 и вызываем C-обработчик
    mv a0, sp
    jal ra, trap_handler

    # Восстанавливаем пользовательский sp в mscratch (на случай если он изменился)
    ld t0,   240(sp)
    csrw mscratch, t0

    # Восстанавливаем регистры общего назначения
    ld ra,   0(sp)
    ld gp,   8(sp)
    ld tp,   16(sp)
    ld t0,   24(sp)
    ld t1,   32(sp)
    ld t2,   40(sp)
    ld s0,   48(sp)
    ld s1,   56(sp)
    ld a0,   64(sp)
    ld a1,   72(sp)
    ld a2,   80(sp)
    ld a3,   88(sp)
    ld a4,   96(sp)
    ld a5,   104(sp)
    ld a6,   112(sp)
    ld a7,   120(sp)
    ld s2,   128(sp)
    ld s3,   136(sp)
    ld s4,   144(sp)
    ld s5,   152(sp)
    ld s6,   160(sp)
    ld s7,   168(sp)
    ld s8,   176(sp)
    ld s9,   184(sp)
    ld s10,  192(sp)
    ld s11,  200(sp)
    ld t3,   208(sp)
    ld t4,   216(sp)
    ld t5,   224(sp)
    ld t6,   232(sp)

    # Освобождаем фрейм на стеке ядра и возвращаем стек в mscratch, а U-sp в sp
    addi sp, sp, 256
    csrrw sp, mscratch, sp

    mret

# ============================================================================
# ОПУСКАНИЕ В USER MODE: void enter_user_mode(uintptr_t entry, uintptr_t user_sp)
# a0 = адрес точки входа функции в U-mode, a1 = вершина стека пользователя
# ============================================================================
enter_user_mode:
    # Сохраняем адрес возврата и callee-saved регистры ядра, чтобы SYS_EXIT вернулся в shell
    addi sp, sp, -112
    sd ra,  0(sp)
    sd s0,  8(sp)
    sd s1,  16(sp)
    sd s2,  24(sp)
    sd s3,  32(sp)
    sd s4,  40(sp)
    sd s5,  48(sp)
    sd s6,  56(sp)
    sd s7,  64(sp)
    sd s8,  72(sp)
    sd s9,  80(sp)
    sd s10, 88(sp)
    sd s11, 96(sp)

    # Сохраняем текущий sp ядра в mscratch для возврата при ловушке
    csrw mscratch, sp

    # Настраиваем mstatus: сбрасываем биты MPP (биты 12:11 = 00 -> U-mode), ставим MPIE (бит 7 = 1)
    csrr t0, mstatus
    li t1, ~(3 << 11)
    and t0, t0, t1
    ori t0, t0, (1 << 7)
    csrw mstatus, t0

    # Записываем адрес точки входа U-mode в mepc и переключаем sp на стек пользователя
    csrw mepc, a0
    mv sp, a1

    # Аппаратный переход в User Mode!
    mret

# Точка возврата в ядро при вызове системного вызова SYS_EXIT из U-mode
.global exit_user_mode_return
exit_user_mode_return:
    # В a0 передан код возврата процесса; текущий sp указывает на сохранённый фрейм enter_user_mode
    ld ra,  0(sp)
    ld s0,  8(sp)
    ld s1,  16(sp)
    ld s2,  24(sp)
    ld s3,  32(sp)
    ld s4,  40(sp)
    ld s5,  48(sp)
    ld s6,  56(sp)
    ld s7,  64(sp)
    ld s8,  72(sp)
    ld s9,  80(sp)
    ld s10, 88(sp)
    ld s11, 96(sp)
    addi sp, sp, 112
    ret

```


main.h
```c
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

// Макрос чтения CSR
#define read_csr(reg)                                                                                                  \
    ({                                                                                                                 \
        uint64_t __tmp;                                                                                                \
        asm volatile("csrr %0, " #reg : "=r"(__tmp));                                                                  \
        __tmp;                                                                                                         \
    })


void trap_handler(trap_frame_t* frame);
void enter_user_mode(uintptr_t entry, uintptr_t user_sp);
void exit_user_mode_return(void);
void run_user_mode_demo(void);

```

main.c
```

/**
 * @brief Обёртка пользовательского пространства над инструкцией ecall.
 * Выполняется в U-mode: прямая запись в UART0 здесь вызвала бы Load/Store Fault
 * при включённом PMP, поэтому обращение к ядру идёт строго через ABI ecall.
 */
static uint64_t sys_call(uint64_t sys_num, uint64_t arg0, uint64_t arg1) {
    register uint64_t a0 asm("a0") = arg0;
    register uint64_t a1 asm("a1") = arg1;
    register uint64_t a7 asm("a7") = sys_num;

    asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a7) : "memory");
    return a0;
}

/**
 * @brief Тестовая задача, исполняемая процессором строго в User Mode (U-mode).
 */
static void user_task(void) {
    const char* msg = "[U-MODE] Привет из непривилегированного пространства! Запрашиваю тики...\n";
    sys_call(SYS_PRINT, (uint64_t)msg, 0);

    uint64_t ticks = sys_call(SYS_GET_TICKS, 0, 0);
    if (ticks > 0) {
        sys_call(SYS_PRINT, (uint64_t) "[U-MODE] Системный вызов SYS_GET_TICKS вернул ненулевое время!\n", 0);
    }

    sys_call(SYS_PRINT, (uint64_t) "[U-MODE] Вызываю SYS_EXIT(0) для возврата в ядро...\n", 0);
    sys_call(SYS_EXIT, 0, 0);

    for (;;) {
    }
}

/**
 * @brief Центральный диспетчер синхронных исключений и прерываний ядра.
 */
void trap_handler(trap_frame_t* frame) {
    uint64_t mcause = read_csr(mcause);
    uint64_t mepc = read_csr(mepc);
    uint64_t mtval = read_csr(mtval);

    // Проверяем старший бит: 1 = аппаратное прерывание, 0 = синхронное исключение
    if ((mcause >> 63) != 0) {
        kprintf("[TRAP] Аппаратное прерывание: mcause=0x%lx, mepc=0x%lx\n", mcause, mepc);
        return;
    }

    if (mcause == CAUSE_ECALL_FROM_UMODE) {
        uint64_t sys_num = frame->a7;

        switch (sys_num) {
        case SYS_PRINT: {
            const char* str = (const char*)frame->a0;
            if (str != NULL) {
                print(str);
            }
            frame->a0 = 0;
            break;
        }
        case SYS_GET_TICKS: {
            frame->a0 = get_ticks();
            break;
        }
        case SYS_EXIT: {
            uint64_t exit_code = frame->a0;
            kprintf("[KERNEL] Процесс U-mode завершился через SYS_EXIT (код: %lu)\n", exit_code);
            // Фрейм enter_user_mode находится ровно над 256-байтным trap_frame_t
            frame->sp = (uintptr_t)frame + sizeof(trap_frame_t);
            write_csr(mepc, (uint64_t)exit_user_mode_return);
            uint64_t mstatus = read_csr(mstatus);
            mstatus |= MSTATUS_MPP_MASK; // Возвращаемся обратно в Machine Mode (MPP = 11)
            write_csr(mstatus, mstatus);
            return;
        }
        // case SYS_EXIT: {
        //     uint64_t exit_code = frame->a0;
        //     kprintf("[KERNEL] Процесс U-mode завершился через SYS_EXIT (код: %lu)\n", exit_code);
        //     // Перенаправляем mret в точку возврата ядра exit_user_mode_return
        //     uint64_t kernel_sp = read_csr(mscratch);
        //     frame->sp = kernel_sp;
        //     write_csr(mepc, (uint64_t)exit_user_mode_return);
        //     uint64_t mstatus = read_csr(mstatus);
        //     mstatus |= MSTATUS_MPP_MASK; // Возвращаемся обратно в Machine Mode (MPP = 11)
        //     write_csr(mstatus, mstatus);
        //     return;
        // }
        default:
            kprintf("[SECURITY] Неизвестный syscall %lu из U-mode (mepc=0x%lx)\n", sys_num, mepc);
            frame->a0 = (uint64_t)-1;
            break;
        }

        // Инструкция ecall занимает 4 байта — сдвигаем mepc на следующую инструкцию
        write_csr(mepc, mepc + 4);
        return;
    }

    kprintf("\n[PANIC] Необработанное исключение: mcause=%lu, mepc=0x%lx, mtval=0x%lx\n", mcause, mepc, mtval);
    for (;;) {
        asm volatile("wfi");
    }
}

/**
 * @brief Выделяет пользовательский стек в куче и запускает user_task в U-mode.
 */
void run_user_mode_demo(void) {
    const size_t user_stack_size = 4096;
    uint8_t* user_stack = (uint8_t*)kmalloc(user_stack_size);
    if (user_stack == NULL) {
        kprintf("[ERROR] Не удалось выделить память под стек U-mode\n");
        return;
    }

    // Временное открывающее правило PMP TOR на всё пространство (на Этапе 10 сузим до границ)
    write_csr(pmpaddr0, (~0ULL) >> 10);
    write_csr(pmpcfg0, 0x0FULL);

    uintptr_t user_sp = (uintptr_t)(user_stack + user_stack_size);
    kprintf("[KERNEL] Опускаем процессор в U-mode (entry=0x%lx, user_sp=0x%lx)...\n", (uintptr_t)user_task, user_sp);

    enter_user_mode((uintptr_t)user_task, user_sp);

    kprintf("[KERNEL] Управление успешно возвращено в M-mode Shell!\n");
    kfree(user_stack);
}


// * | usermode      | Запустить задачу в User Mode (U-mode + ecall)     |

void execute_command(const char* cmd) {

    kprintf("  usermode     - Запустить задачу в User Mode (U-mode + ecall)\n");

    {

    } else if (mystrcmp(cmd, "usermode") == 0) {
        run_user_mode_demo();

    }
}
```