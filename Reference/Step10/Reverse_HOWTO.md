linker.d
```
  . = ORIGIN(ram);
  _text_start = .;

  .text : {
    *(.text.init)
    *(.text .text.*)
  } > ram :text

  .rodata : {
    *(.rodata .rodata.*)
    . = ALIGN(4);
    _text_end = .;
  } > ram :text

```

main.h
```c
// NOLINTBEGIN(bugprone-reserved-identifier)
extern char _text_start[];
extern char _text_end[];
extern char _heap_start[];
extern char _heap_end[];
// NOLINTEND(bugprone-reserved-identifier)



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



void pmp_configure_user(uintptr_t ustack_base, uintptr_t ustack_top);
void run_pmp_violation_demo(void);
```


main.c
```c
/**
 * @brief Настраивает аппаратную защиту памяти PMP (режим TOR) для процесса U-mode.
 *
 * Разрешает U-mode:
 *   - R+X для кода и констант [_text_start, _text_end)
 *   - R+W (без X, аппаратный NX) для собственного стека [ustack_base, ustack_top)
 * Запрещает прямой доступ к памяти ядра (.data, .bss, куча) и MMIO (UART0, CLINT).
 */
void pmp_configure_user(uintptr_t ustack_base, uintptr_t ustack_top) {
    uintptr_t text_start = (uintptr_t)_text_start;
    uintptr_t text_end = (uintptr_t)_text_end;

    write_csr(pmpaddr0, text_start >> 2);
    write_csr(pmpaddr1, text_end >> 2);
    write_csr(pmpaddr2, ustack_base >> 2);
    write_csr(pmpaddr3, ustack_top >> 2);

    // Формируем 64-битный pmpcfg0 (по 1 байту на запись pmp0..pmp3):
    // cfg0 = 0 (OFF, нижняя граница)
    // cfg1 = PMP_TOR | PMP_R | PMP_X (код + rodata)
    // cfg2 = PMP_TOR (запретная зона памяти ядра и чужих стеков)
    // cfg3 = PMP_TOR | PMP_R | PMP_W (стек задачи, NX)
    uint64_t cfg = ((PMP_TOR | PMP_R | PMP_X) << 8) | (PMP_TOR << 16) | ((PMP_TOR | PMP_R | PMP_W) << 24);
    write_csr(pmpcfg0, cfg);

    // Сбрасываем кэш трансляций после обновления правил PMP
    asm volatile("sfence.vma zero, zero" ::: "memory");
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

        pmp_configure_user((uintptr_t)tasks[next_idx].ustack, (uintptr_t)(tasks[next_idx].ustack + 4096));

        if (tasks[next_idx].state == TASK_READY) {
            current_task_idx = next_idx;
            tasks[next_idx].state = TASK_RUNNING;
            write_csr(mepc, tasks[next_idx].mepc);
            return tasks[next_idx].frame;
        }
    }
    return current_frame;
}
// # trap_frame_t* trap_handler(trap_frame_t* frame) {

    if (mcause == CAUSE_INSTRUCTION_ACCESS_FAULT || mcause == CAUSE_LOAD_ACCESS_FAULT ||
        mcause == CAUSE_STORE_ACCESS_FAULT) {
        const char* fault_type = "Instruction Access Fault (NX / Exec запрещён)";
        if (mcause == CAUSE_LOAD_ACCESS_FAULT) {
            fault_type = "Load Access Fault (Чтение запрещённой области)";
        } else if (mcause == CAUSE_STORE_ACCESS_FAULT) {
            fault_type = "Store/AMO Access Fault (Запись в защищённую область / MMIO)";
        }

        kprintf("\n[SECURITY] PMP Violation! %s\n", fault_type);
        kprintf("[SECURITY] mcause=%lu | mepc=0x%lx | mtval=0x%lx — процесс принудительно завершён!\n", mcause, mepc,
                mtval);

        // Аварийно завершаем нарушивший U-mode процесс и возвращаемся в M-mode Shell
        frame->a0 = (uint64_t)-1;
        frame->sp = (uintptr_t)frame + sizeof(trap_frame_t);
        write_csr(mepc, (uint64_t)exit_user_mode_return);
        uint64_t mstatus = read_csr(mstatus);
        mstatus |= MSTATUS_MPP_MASK;
        write_csr(mstatus, mstatus);
        return frame;
    }


// run_sheduler_demo

    // Этап 10: сужаем границы
    pmp_configure_user((uintptr_t)tasks[0].ustack, tasks[0].frame->sp);

    // write_csr(pmpaddr0, (~0ULL) >> 10);
    // write_csr(pmpcfg0, 0x0FULL);


/**
 * @brief Злонамеренная задача U-mode: пытается напрямую писать в UART0 в обход syscall.
 */
static void rogue_uart_task(void) {
    sys_call(SYS_PRINT, (uint64_t) "[U-MODE | ROGUE] Пытаюсь напрямую записать символ в UART0 (0x10000000)...\n", 0);
    *UART0_THR = 'X'; // Должно вызвать Store Access Fault (mcause = 7)!
    sys_call(SYS_PRINT, (uint64_t) "[U-MODE | ROGUE] ОШИБКА: запись в UART0 прошла без блокировки!\n", 0);
    sys_call(SYS_EXIT, 0, 0);
}

void run_pmp_violation_demo(void) {
    const size_t user_stack_size = 4096;
    uint8_t* user_stack = (uint8_t*)kmalloc(user_stack_size);
    if (user_stack == NULL) {
        kprintf("[ERROR] Не удалось выделить память под стек U-mode\n");
        return;
    }

    uintptr_t user_sp = (uintptr_t)(user_stack + user_stack_size);
    pmp_configure_user((uintptr_t)user_stack, user_sp);

    kprintf("[KERNEL] Запускаем задачу-нарушитель U-mode со строгим PMP...\n");
    enter_user_mode((uintptr_t)rogue_uart_task, user_sp);

    kprintf("[KERNEL] Ядро устояло, управление возвращено в M-mode Shell!\n");
    kfree(user_stack);
}




    // Временное открывающее правило PMP TOR на всё пространство (на Этапе 10 сузим до границ)
    // write_csr(pmpaddr0, (~0ULL) >> 10);
    // write_csr(pmpcfg0, 0x0FULL);

    uintptr_t user_sp = (uintptr_t)(user_stack + user_stack_size);
    kprintf("[KERNEL] Опускаем процессор в U-mode (entry=0x%lx, user_sp=0x%lx)...\n", (uintptr_t)user_task, user_sp);
    pmp_configure_user((uintptr_t)user_stack, user_sp);

    enter_user_mode((uintptr_t)user_task, user_sp);

    kprintf("[KERNEL] Управление успешно возвращено в M-mode Shell!\n");
    kfree(user_stack);

//
// * | pmptest       | Проверка аппаратная защита PMP                    |
void execute_command(const char* cmd) {
        kprintf("  pmptest      - Проверка аппаратной защита PMP");

    } else if (mystrcmp(cmd, "pmptest") == 0) {
        run_pmp_violation_demo();

```