start.S
```
.section .text.init
.global _start
.global trap_vector
.global enter_user_mode

_start:
 # 1. Отключаем все ядра, кроме основного (hart 0)
 csrr a0, mhartid # Прочитать ID текущего ядра
 bnez a0, park # Если это не ядро 0, отправляем в бесконечный цикл

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
# .align 4
# trap_vector:
# # Атомарно меняем местами sp (пользовательский) и mscratch (стек ядра)
# csrrw sp, mscratch, sp

# # Выделяем 256 байт (32 регистра * 8 байт) на стеке ядра для сохранения контекста
# addi sp, sp, -256

# sd ra, 0(sp)
# sd gp, 8(sp)
# sd tp, 16(sp)
# sd t0, 24(sp)
# sd t1, 32(sp)
# sd t2, 40(sp)
# sd s0, 48(sp)
# sd s1, 56(sp)
# sd a0, 64(sp)
# sd a1, 72(sp)
# sd a2, 80(sp)
# sd a3, 88(sp)
# sd a4, 96(sp)
# sd a5, 104(sp)
# sd a6, 112(sp)
# sd a7, 120(sp)
# sd s2, 128(sp)
# sd s3, 136(sp)
# sd s4, 144(sp)
# sd s5, 152(sp)
# sd s6, 160(sp)
# sd s7, 168(sp)
# sd s8, 176(sp)
# sd s9, 184(sp)
# sd s10, 192(sp)
# sd s11, 200(sp)
# sd t3, 208(sp)
# sd t4, 216(sp)
# sd t5, 224(sp)
# sd t6, 232(sp)

# # Сохраняем исходный пользовательский sp (сейчас лежит в mscratch) во фрейм
# csrr t0, mscratch
# sd t0, 240(sp)

# # Передаём указатель на trap_frame_t в a0 и вызываем C-обработчик
# mv a0, sp
# jal ra, trap_handler

# # Восстанавливаем пользовательский sp в mscratch (на случай если он изменился)
# ld t0, 240(sp)
# csrw mscratch, t0

# # Восстанавливаем регистры общего назначения
# ld ra, 0(sp)
# ld gp, 8(sp)
# ld tp, 16(sp)
# ld t0, 24(sp)
# ld t1, 32(sp)
# ld t2, 40(sp)
# ld s0, 48(sp)
# ld s1, 56(sp)
# ld a0, 64(sp)
# ld a1, 72(sp)
# ld a2, 80(sp)
# ld a3, 88(sp)
# ld a4, 96(sp)
# ld a5, 104(sp)
# ld a6, 112(sp)
# ld a7, 120(sp)
# ld s2, 128(sp)
# ld s3, 136(sp)
# ld s4, 144(sp)
# ld s5, 152(sp)
# ld s6, 160(sp)
# ld s7, 168(sp)
# ld s8, 176(sp)
# ld s9, 184(sp)
# ld s10, 192(sp)
# ld s11, 200(sp)
# ld t3, 208(sp)
# ld t4, 216(sp)
# ld t5, 224(sp)
# ld t6, 232(sp)

# # Освобождаем фрейм на стеке ядра и возвращаем стек в mscratch, а U-sp в sp
# addi sp, sp, 256
# csrrw sp, mscratch, sp

# mret

.align 4
trap_vector:
 # Атомарно меняем местами sp (пользовательский) и mscratch (стек ядра)
 csrrw sp, mscratch, sp

 # Выделяем 256 байт (32 регистра * 8 байт) на стеке ядра для сохранения контекста
 addi sp, sp, -256

 sd ra, 0(sp)
 sd gp, 8(sp)
 sd tp, 16(sp)
 sd t0, 24(sp)
 sd t1, 32(sp)
 sd t2, 40(sp)
 sd s0, 48(sp)
 sd s1, 56(sp)
 sd a0, 64(sp)
 sd a1, 72(sp)
 sd a2, 80(sp)
 sd a3, 88(sp)
 sd a4, 96(sp)
 sd a5, 104(sp)
 sd a6, 112(sp)
 sd a7, 120(sp)
 sd s2, 128(sp)
 sd s3, 136(sp)
 sd s4, 144(sp)
 sd s5, 152(sp)
 sd s6, 160(sp)
 sd s7, 168(sp)
 sd s8, 176(sp)
 sd s9, 184(sp)
 sd s10, 192(sp)
 sd s11, 200(sp)
 sd t3, 208(sp)
 sd t4, 216(sp)
 sd t5, 224(sp)
 sd t6, 232(sp)

 # Сохраняем исходный пользовательский sp (сейчас лежит в mscratch) во фрейм
 csrr t0, mscratch
 sd t0, 240(sp)

 # Передаём текущий trap_frame_t* в a0; C-обработчик вернёт trap_frame_t* следующего потока
 mv a0, sp
 jal ra, trap_handler
 mv sp, a0

 # Запоминаем вершину стека ядра для следующего входа в trap_vector
 addi t0, sp, 256
 csrw mscratch, t0

 # Загружаем пользовательский sp выбранного потока в mscratch для финального обмена
 ld t0, 240(sp)
 csrw mscratch, t0

 # Восстанавливаем регистры общего назначения
 ld ra, 0(sp)
 ld gp, 8(sp)
 ld tp, 16(sp)
 ld t0, 24(sp)
 ld t1, 32(sp)
 ld t2, 40(sp)
 ld s0, 48(sp)
 ld s1, 56(sp)
 ld a0, 64(sp)
 ld a1, 72(sp)
 ld a2, 80(sp)
 ld a3, 88(sp)
 ld a4, 96(sp)
 ld a5, 104(sp)
 ld a6, 112(sp)
 ld a7, 120(sp)
 ld s2, 128(sp)
 ld s3, 136(sp)
 ld s4, 144(sp)
 ld s5, 152(sp)
 ld s6, 160(sp)
 ld s7, 168(sp)
 ld s8, 176(sp)
 ld s9, 184(sp)
 ld s10, 192(sp)
 ld s11, 200(sp)
 ld t3, 208(sp)
 ld t4, 216(sp)
 ld t5, 224(sp)
 ld t6, 232(sp)

 # Освобождаем фрейм на стеке ядра и возвращаем U-sp в sp, а стек ядра в mscratch
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
 sd ra, 0(sp)
 sd s0, 8(sp)
 sd s1, 16(sp)
 sd s2, 24(sp)
 sd s3, 32(sp)
 sd s4, 40(sp)
 sd s5, 48(sp)
 sd s6, 56(sp)
 sd s7, 64(sp)
 sd s8, 72(sp)
 sd s9, 80(sp)
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
 ld ra, 0(sp)
 ld s0, 8(sp)
 ld s1, 16(sp)
 ld s2, 24(sp)
 ld s3, 32(sp)
 ld s4, 40(sp)
 ld s5, 48(sp)
 ld s6, 56(sp)
 ld s7, 64(sp)
 ld s8, 72(sp)
 ld s9, 80(sp)
 ld s10, 88(sp)
 ld s11, 96(sp)
 addi sp, sp, 112
 ret
```

Обратите внимание: строки addi t0, sp, 256; csrw mscratch, t0 выше перезаписывались следующей парой ld t0, 240(sp); csrw mscratch, t0 — оставьте только вторую пару ld t0, 240(sp); csrw mscratch, t0, так как финальный addi sp, sp, 256; csrrw sp, mscratch, sp сам положит sp + 256 в mscratch!

main.h
```c
// Регистры аппаратного таймера CLINT на плате QEMU virt (Hart 0)
#define CLINT_BASE       0x02000000UL
#define CLINT_MTIMECMP0  ((volatile uint64_t*)(CLINT_BASE + 0x4000))
#define CLINT_MTIME      ((volatile uint64_t*)(CLINT_BASE + 0xBFF8))

// Квант времени планировщика: 100 мс при частоте таймера 10 МГц (1 000 000 тиков)
#define SCHED_QUANTUM_TICKS (TIMER_FREQ / 10)

// Код прерывания машинного таймера (старший бит 63 = 1, код = 7)
#define MCAUSE_TIMER_INTERRUPT ((1ULL << 63) | 7ULL)

// Состояния потока и дескриптор задачи (Task Control Block)
#define MAX_TASKS 4

typedef enum {
    TASK_UNUSED = 0,
    TASK_READY  = 1,
    TASK_RUNNING = 2,
    TASK_EXITED = 3
} task_state_t;

typedef struct {
    int id;
    task_state_t state;
    uint64_t mepc;          // Сохранённый счётчик команд потока
    uint8_t* kstack;        // Выделенный в куче стек ядра (на вершине лежит trap_frame_t)
    uint8_t* ustack;        // Выделенный в куче стек пользователя
    trap_frame_t* frame;    // Указатель на контекст 32 регистров в kstack
} task_t;

// ------ некий код ----

trap_frame_t* trap_handler(trap_frame_t* frame);
void run_scheduler_demo(void);

```

main.c
```c
static task_t tasks[MAX_TASKS];
static int current_task_idx = -1;
static int scheduler_active = 0;
static int timer_ticks_count = 0;
static uintptr_t shell_return_sp = 0;

/**
 * @brief Планирует следующее прерывание таймера CLINT через quantum тиков.
 */
static void timer_set_next(uint64_t quantum) {
    *CLINT_MTIMECMP0 = *CLINT_MTIME + quantum;
}

/**
 * @brief Включает или отключает прерывания машинного таймера (MIE.MTIE).
 */
static void timer_enable(bool enable) {
    if (enable) {
        timer_set_next(SCHED_QUANTUM_TICKS);
        set_csr(mie, MIE_MTIE);
    } else {
        clear_csr(mie, MIE_MTIE);
    }
}

/**
 * @brief Поток А (U-mode): выполняет вычисления без добровольной уступки CPU.
 */
static void worker_task_a(void) {
    uint64_t iter = 0;
    for (;;) {
        if ((iter % 2000000ULL) == 0) {
            sys_call(SYS_PRINT, (uint64_t)"  [TASK A | U-mode] Выполняю вычисления...\n", 0);
        }
        iter++;
    }
}

/**
 * @brief Поток Б (U-mode): конкурирует за процессорное время с Потоком А.
 */
static void worker_task_b(void) {
    uint64_t iter = 0;
    for (;;) {
        if ((iter % 2000000ULL) == 0) {
            sys_call(SYS_PRINT, (uint64_t)"  [TASK B | U-mode] Параллельный поток активен!\n", 0);
        }
        iter++;
    }
}

/**
 * @brief Инициализирует слот задачи в планировщике и формирует начальный trap_frame_t.
 */
static bool task_create(int slot, void (*entry)(void)) {
    const size_t stack_size = 4096;
    uint8_t* kstack = (uint8_t*)kmalloc(stack_size);
    uint8_t* ustack = (uint8_t*)kmalloc(stack_size);
    if (kstack == NULL || ustack == NULL) {
        kfree(kstack);
        kfree(ustack);
        return false;
    }

    // Размещаем начальный 256-байтный контекст на вершине стека ядра задачи
    trap_frame_t* frame = (trap_frame_t*)(kstack + stack_size - sizeof(trap_frame_t));
    uint8_t* raw = (uint8_t*)frame;
    for (size_t i = 0; i < sizeof(trap_frame_t); i++) {
        raw[i] = 0;
    }
    frame->sp = (uintptr_t)(ustack + stack_size);

    tasks[slot].id = slot + 1;
    tasks[slot].state = TASK_READY;
    tasks[slot].mepc = (uintptr_t)entry;
    tasks[slot].kstack = kstack;
    tasks[slot].ustack = ustack;
    tasks[slot].frame = frame;
    return true;
}

/**
 * @brief Выбирает следующую готовую задачу по кругу (Round-Robin).
 */
static trap_frame_t* schedule_next(trap_frame_t* current_frame) {
    if (current_task_idx >= 0 && tasks[current_task_idx].state == TASK_RUNNING) {
        tasks[current_task_idx].mepc = read_csr(mepc);
        tasks[current_task_idx].frame = current_frame;
        tasks[current_task_idx].state = TASK_READY;
    }

    for (int offset = 1; offset <= MAX_TASKS; offset++) {
        int next_idx = (current_task_idx + offset) % MAX_TASKS;
        if (tasks[next_idx].state == TASK_READY) {
            current_task_idx = next_idx;
            tasks[next_idx].state = TASK_RUNNING;
            write_csr(mepc, tasks[next_idx].mepc);
            return tasks[next_idx].frame;
        }
    }
    return current_frame;
}

```
Надо обновить trap_handler() (теперь возвращает trap_frame_t*), а также добавить run_scheduler_demo() и команду sched в execute_command():

main.c
```c
trap_frame_t* trap_handler(trap_frame_t* frame) {
    uint64_t mcause = read_csr(mcause);
    uint64_t mepc = read_csr(mepc);
    uint64_t mtval = read_csr(mtval);

    // 1. Обработка прерывания машинного таймера CLINT (Вытесняющий Round-Robin)
    if (mcause == MCAUSE_TIMER_INTERRUPT) {
        timer_set_next(SCHED_QUANTUM_TICKS);

        if (scheduler_active) {
            timer_ticks_count++;
            kprintf("[TIMER IRQ #%d] Переключение контекста (Round-Robin)...\n", timer_ticks_count);

            // После 6 квантов (600 мс) завершаем демонстрацию и возвращаемся в Shell
            if (timer_ticks_count >= 6) {
                timer_enable(false);
                scheduler_active = 0;
                frame->sp = shell_return_sp;
                write_csr(mepc, (uint64_t)exit_user_mode_return);
                uint64_t mstatus = read_csr(mstatus);
                mstatus |= MSTATUS_MPP_MASK;
                write_csr(mstatus, mstatus);
                return frame;
            }
            return schedule_next(frame);
        }
        return frame;
    }

    if ((mcause >> 63) != 0) {
        kprintf("[TRAP] Аппаратное прерывание: mcause=0x%lx, mepc=0x%lx\n", mcause, mepc);
        return frame;
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
            frame->sp = (uintptr_t)frame + sizeof(trap_frame_t);
            write_csr(mepc, (uint64_t)exit_user_mode_return);
            uint64_t mstatus = read_csr(mstatus);
            mstatus |= MSTATUS_MPP_MASK;
            write_csr(mstatus, mstatus);
            return frame;
        }
        default:
            kprintf("[SECURITY] Неизвестный syscall %lu из U-mode (mepc=0x%lx)\n", sys_num, mepc);
            frame->a0 = (uint64_t)-1;
            break;
        }

        write_csr(mepc, mepc + 4);
        return frame;
    }

    kprintf("\n[PANIC] Необработанное исключение: mcause=%lu, mepc=0x%lx, mtval=0x%lx\n", mcause, mepc, mtval);
    for (;;) {
        asm volatile("wfi");
    }
}

void run_scheduler_demo(void) {
    for (int i = 0; i < MAX_TASKS; i++) {
        tasks[i].state = TASK_UNUSED;
        tasks[i].kstack = NULL;
        tasks[i].ustack = NULL;
    }

    if (!task_create(0, worker_task_a) || !task_create(1, worker_task_b)) {
        kprintf("[ERROR] Не удалось выделить память под задачи планировщика\n");
        kfree(tasks[0].kstack);
        kfree(tasks[0].ustack);
        return;
    }

    write_csr(pmpaddr0, (~0ULL) >> 10);
    write_csr(pmpcfg0, 0x0FULL);

    current_task_idx = 0;
    tasks[0].state = TASK_RUNNING;
    timer_ticks_count = 0;
    scheduler_active = 1;

    // Запоминаем вершину стека ядра для возврата в Shell после завершения демо
    uint64_t cur_sp;
    asm volatile("mv %0, sp" : "=r"(cur_sp));
    shell_return_sp = cur_sp - 112; // Учитываем фрейм enter_user_mode

    kprintf("[SCHED] Запуск вытесняющего планировщика Round-Robin (2 потока U-mode, квант 100 мс)...\n");
    timer_enable(true);

    enter_user_mode(tasks[0].mepc, tasks[0].frame->sp);

    kprintf("[SCHED] Демонстрация планировщика завершена. Освобождаем память задач...\n");
    for (int i = 0; i < 2; i++) {
        kfree(tasks[i].kstack);
        kfree(tasks[i].ustack);
    }
}
```
