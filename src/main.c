
/**
 * @file kernel.c
 * @brief Минимальное микроядро RISC-V (RV64GC) для QEMU virt с интерактивным shell.
 *
 * Реализует UART-ввод/вывод, форматированный вывод (kprintf), чтение
 * аппаратного таймера и обработку команд пользователя. Сборка — bare-metal,
 * без стандартной библиотеки.
 */

#include "./main.h"
#include "./include/mm.h"
#include "./include/uart.h"
#include "./include/csr.h"
#include "./include/kernel_layout.h"
#include "./include/string.h"

#include <stdarg.h>

static task_t tasks[MAX_TASKS];
static int current_task_idx = -1;
static int scheduler_active = 0;
static int timer_ticks_count = 0;
static uintptr_t shell_return_sp = 0;


/**
 * @brief Настраивает аппаратную защиту памяти PMP (режим TOR) для процесса U-mode.
 *
 * Разрешает U-mode:
 *   - R+X для кода и констант [_text_start, _text_end)
 *   - R+W (без X, аппаратный NX) для собственного стека [ustack_base, ustack_top)
 * Запрещает прямой доступ к памяти ядра (.data, .bss, куча) и MMIO (UART0, CLINT).
 */
void pmp_configure_user(uintptr_t ustack_base, uintptr_t ustack_top) {
    // entry0: [0, _text_start)        — запрет (MMIO: UART, CLINT)
    // entry1: [_text_start, _rodata_end) — R+X (код + константы, читаемые U-mode)
    // entry2: [_rodata_end, ustack_base) — запрет (.data, .bss, куча)
    // entry3: [ustack_base, ustack_top)  — R+W, NX (стек задачи)
    // entry4: [ustack_top, конец RAM)    — запрет (куча выше, стеки других задач)

    write_csr(pmpaddr0, (uintptr_t)_text_start >> 2);
    write_csr(pmpaddr1, (uintptr_t)_text_end >> 2);
    write_csr(pmpaddr2, ustack_base >> 2);
    write_csr(pmpaddr3, ustack_top >> 2);
    write_csr(pmpaddr4, ((uintptr_t)_ram_end - 1) >> 2);

    uint64_t cfg = (PMP_TOR)                          |   // entry0: низ закрыт
                   ((PMP_TOR | PMP_R | PMP_X) << 8)   |   // entry1: код
                   ((PMP_TOR) << 16)                  |   // entry2: запрет ядра
                   ((PMP_TOR | PMP_R | PMP_W) << 24)  |   // entry3: стек задачи
                   ((PMP_TOR) << 32);                     // entry4: верх закрыт
    write_csr(pmpcfg0, cfg);

    // Сброс трансляций после изменения правил защиты
    asm volatile("sfence.vma zero, zero" ::: "memory");
}



/**
 * @brief Включает архитектурные прерывания процессора в Machine-mode.
 *
 * Последовательность инициализации:
 * 1. Полностью очищает регистр @c mie — ни одно устройство не побеспокоит ядро.
 * 2. Сбрасывает отложенные флаги в @c mip, оставшиеся от загрузчика.
 * 3. Устанавливает глобальный бит @c MIE в @c mstatus.
 *
 * @warning Регистр обработчика прерываний (mtvec) не настраивается здесь —
 *          до его инициализации включать MSTATUS_MIE опасно.
 */
void enable_interrupts(void) {

    /**

    Структура основных битов регистра mip

    Стандартные биты соответствуют типам программных, таймерных и внешних прерываний для разных уровней привилегий
    (Machine и Supervisor):

    Бит	Поле	Описание	Доступность / Запись
    1	SSIP	Supervisor Software Interrupt Pending (прерывание программного обеспечения уровня Supervisor)	Чтение /
    Запись (при наличии режима S) 3	MSIP	Machine Software Interrupt Pending (прерывание программного обеспечения
    уровня Machine)	Чтение / Запись 5	STIP	Supervisor Timer Interrupt Pending (таймерное прерывание уровня
    Supervisor)	Чтение / Запись (или управляется таймером) 7	MTIP	Machine Timer Interrupt Pending (таймерное
    прерывание уровня Machine)	Обычно чтение (устанавливается таймером M-mode, например CLINT) 9	SEIP	Supervisor
    External Interrupt Pending (внешнее прерывание уровня Supervisor)	Чтение / Запись или управляется PLIC/APLIC 11
    MEIP	Machine External Interrupt Pending (внешнее прерывание уровня Machine)	Обычно чтение (устанавливается
    контроллером прерываний)

    Примечание: В расширенных спецификациях (например, с поддержкой LCOFI или виртуализации) могут присутствовать и
    другие биты.

    Как это работает на практике

    1. Когда периферийное устройство, таймер или программный триггер вызывают прерывание, соответствующий бит в регистре
    mip устанавливается в 1.
    2. Чтобы прерывание реально обработалось процессором, должны совпасть три условия:
        • Глобальное разрешение прерываний в регистре mstatus (бит MIE для M-mode).
        • Соответствующий бит разрешения в регистре mie должен быть равен 1.
        • Бит ожидания в регистре mip должен быть равен 1.
    Подробную спецификацию привилегированного уровня можно найти в официальной библиотеке RISC-V Privileged
    Architecture.

    */

    // 1. Принудительно гасим ВСЕ разрешения прерываний (таймер, софтверные, внешние)
    // Записываем чистый 0 в регистр mie. Теперь ни одно устройство не побеспокоит процессор.
    write_csr(mie, 0);

    // 2. Сбрасываем (очищаем) все отложенные флаги прерываний в регистре mip.
    // Запись нуля в mip сбрасывает незавершенные аппаратные запросы, оставшиеся от загрузчика.
    write_csr(mip, 0);

    // 3. Теперь, когда периферия гарантированно молчит, безопасно включаем
    // архитектурную готовность ядра обрабатывать прерывания глобально.
    set_csr(mstatus, MSTATUS_MIE);
}




/**
 * @brief Читает 64-битный счётчик аппаратного времени (mtime).
 * @return Текущее значение счётчика в тиках.
 *
 * В QEMU virt тики идут с частотой TIMER_FREQ (10 МГц).
 * Инструкция rdtime атомарно читает CSR time, так что разрыв
 * между старшим и младшим словом невозможен.
 */
uint64_t get_ticks(void) {
    uint64_t ticks;
    asm volatile("rdtime %0" : "=r"(ticks));
    return ticks;
}

/**
 * @brief Вычисляет и выводит время работы ядра в формате MM:SS.mmm.
 *
 * Аппаратные тики (rdtime) переводятся в миллисекунды делением на
 * TIMER_FREQ / 1000. Часы не отображаются, только минуты и секунды.
 * В конце печатается счётчик тиков (при больших аптаймах приводит
 * к переполнению при касте к int — чисто косметика для отладки).
 */
void print_uptime(void) {
    uint64_t ticks = get_ticks();

    // Переводим тики в миллисекунды
    uint64_t total_ms = ticks / (TIMER_FREQ / 1000);

    uint64_t ms = total_ms % 1000;
    uint64_t seconds = (total_ms / 1000) % 60;
    uint64_t minutes = (total_ms / 60000);

    kprintf("Uptime: ");
    if (minutes < 10) uart_putc('0');
    kprintf("%d:", (int)minutes);

    if (seconds < 10) uart_putc('0');
    kprintf("%d.", (int)seconds);

    if (ms < 100) uart_putc('0');
    if (ms < 10) uart_putc('0');
    kprintf("%d (Ticks: %d)\n", (int)ms, (int)ticks);
}

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
            sys_call(SYS_PRINT, (uint64_t) "  [TASK A | U-mode] Выполняю вычисления...\n", 0);
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
            sys_call(SYS_PRINT, (uint64_t) "  [TASK B | U-mode] Параллельный поток активен!\n", 0);
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

    // Этап 10: сужаем границы
    pmp_configure_user((uintptr_t)tasks[0].ustack, tasks[0].frame->sp);

    // write_csr(pmpaddr0, (~0ULL) >> 10);
    // write_csr(pmpcfg0, 0x0FULL);

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
// /**
//  * @brief Злонамеренная задача U-mode: пытается напрямую писать в UART0 в обход syscall.
//  */
// static void rogue_uart_task(void) {
//     sys_call(SYS_PRINT, (uint64_t) "[U-MODE | ROGUE] Пытаюсь напрямую записать символ в UART0 (0x10000000)...\n", 0);
//     *UART0_THR = 'X'; // Должно вызвать Store Access Fault (mcause = 7)!
//     sys_call(SYS_PRINT, (uint64_t) "[U-MODE | ROGUE] ОШИБКА: запись в UART0 прошла без блокировки!\n", 0);
//     sys_call(SYS_EXIT, 0, 0);
// }

/**
 * @brief Злонамеренная задача U-mode: пытается напрямую писать в UART0 в обход syscall.
 */
static void rogue_uart_task(void) {
    sys_call(SYS_PRINT, (uint64_t) "[U-MODE | ROGUE] Пытаюсь напрямую записать символ в UART0 (0x10000000)...\n", 0);
    *(volatile uint8_t*)UART0_BASE = 'X';   /* осознанный обход API: должен дать Store Access Fault, mcause=7 */
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
    // write_csr(pmpaddr0, (~0ULL) >> 10);
    // write_csr(pmpcfg0, 0x0FULL);

    uintptr_t user_sp = (uintptr_t)(user_stack + user_stack_size);
    kprintf("[KERNEL] Опускаем процессор в U-mode (entry=0x%lx, user_sp=0x%lx)...\n", (uintptr_t)user_task, user_sp);
    pmp_configure_user((uintptr_t)user_stack, user_sp);

    enter_user_mode((uintptr_t)user_task, user_sp);

    kprintf("[KERNEL] Управление успешно возвращено в M-mode Shell!\n");
    kfree(user_stack);
}

/**
 * @brief Разбирает и выполняет команду, введённую пользователем.
 * @param cmd Указатель на NUL-терминированную строку с командой.
 *
 * Поддерживаемые команды:
 * | Команда       | Действие                                          |
 * |---------------|---------------------------------------------------|
 * | help          | Вывод списка команд                               |
 * | info          | Диагностика ядра (архитектура, UART, SP)          |
 * | clear         | Очистка экрана (ANSI ESC[2J + ESC[H)              |
 * | time, uptime  | Время работы системы                              |
 * | mem           | Показать карту кучи (heap)                        |
 * | memtest       | Выполнить тест аллокатора памяти                  |
 * | usermode      | Запустить задачу в User Mode (U-mode + ecall)     |
 * | sched         | Проверка вытесняющей многозадачности              |
 * | pmptest       | Проверка аппаратная защита PMP                    |
 * | exit, quit    | Завершение работы (poweroff)                      |
 *
 * Пустая строка молча игнорируется; неизвестная команда печатает ошибку.
 */
void execute_command(const char* cmd) {

    // Намеренная ошибка для проверки статического анализатора Cppcheck:
    // Создаем массив на 5 элементов, а пишем в 10-й элемент (выход за границы буфера)
    // int test_array[5];
    // test_array[10] = 42;

    if (kstrcmp(cmd, "help") == 0) {
        kprintf("Доступные команды:\n");
        kprintf("  help         - Вывод этого списка команд\n");
        kprintf("  info         - Вывод системной информации ядра\n");
        kprintf("  clear        - Очистить экран терминала\n");
        kprintf("  time, uptime - Показать время работы системы\n");
        kprintf("  mem          - Показать карту кучи (heap)\n");
        kprintf("  memtest      - Выполнить тест аллокатора памяти\n");
        kprintf("  usermode     - Запустить задачу в User Mode (U-mode + ecall)\n");
        kprintf("  shed         - Проверка вытесняющей многозадачности\n");
        kprintf("  pmptest      - Проверка аппаратной защита PMP\n");
        kprintf("  exit, quit   - Завершение работы программы\n");
    } else if (kstrcmp(cmd, "info") == 0) {
        uint64_t sp_val;
        // Читаем текущий указатель стека sp напрямую из регистра процессора
        asm volatile("mv %0, sp" : "=r"(sp_val));

        kprintf("--- Диагностика ядра %s ---\n", "RISCV");
        kprintf("Архитектура: RV%dGC (64-bit)\n", 64);
        kprintf("Базовый адрес UART0: 0x%x\n", (uint64_t)UART0_BASE);
        kprintf("Текущий указатель стека (SP): 0x%x\n", sp_val);
    } else if (kstrcmp(cmd, "clear") == 0) {
        kprintf("\033[H\033[2J");
    } else if (kstrcmp(cmd, "exit") == 0 || kstrcmp(cmd, "quit") == 0) {
        kprintf("Завершение работы системы...\n");
        poweroff();
    } else if (kstrcmp(cmd, "uptime") == 0 || kstrcmp(cmd, "time") == 0) {
        print_uptime();
    } else if (kstrcmp(cmd, "mem") == 0) {
        heap_dump();
    } else if (kstrcmp(cmd, "memtest") == 0) {
        heap_test();
    } else if (kstrcmp(cmd, "usermode") == 0) {
        run_user_mode_demo();
    } else if (kstrcmp(cmd, "shed") == 0) {
        run_scheduler_demo();
    } else if (kstrcmp(cmd, "pmptest") == 0) {
        run_pmp_violation_demo();
    } else if (kstrlen(cmd) > 0) {
        kprintf("Ошибка: неизвестная команда '%s'. Введите 'help'\n", cmd);
    }
}

/**
 * @brief Точка входа в ядро ОС.
 *
 * Почему void kmain(void), а не int main(void)? Потому что ядро — это мы:
 * стандартная libc-обвязка и точка возврата в "программу" не существуют.
 *
 * Алгоритм работы:
 * 1. Включает прерывания.
 * 2. Печатает баннер и приглашение "sh> ".
 * 3. В бесконечном цикле читает символы из UART, обрабатывая:
 *    - Enter ('\r' или '\n') — выполнить накопленную команду;
 *    - Backspace (8 или 127) — стереть последний символ;
 *    - печатаемые символы — добавить в буфер с эхо-выводом.
 *
 * @warning Буфер ограничен BUFFER_SIZE-1 символами, лишние молча отбрасываются.
 * @note Функция не возвращает управление: единственный путь из цикла — poweroff().
 */
void kmain(void) {
    // uart_init();
    heap_init(); // <-- Обязательно до запуска шелла!

    // Включаем архитектурные прерывания процессора
    enable_interrupts();

    kprintf("\n--- RISCV Микроядро: Shell загружен ---\n");
    kprintf("sh> ");

    char buffer[BUFFER_SIZE];
    int index = 0;

    // Главный цикл ядра ОС
    while (true) { //while (true) более правильно чем while(1)

        char c = uart_getc();

        // Обработка клавиши Enter (\r или \n в зависимости от настроек терминала хоста)
        if (c == '\r' || c == '\n') {
            buffer[index] = '\0'; // Замыкаем строку в буфере
            kprintf("\n");          // Перенос строки на экране

            execute_command(buffer);

            index = 0; // Сбрасываем индекс для новой команды
            kprintf("sh> ");
        }
        // Обработка клавиши Backspace (ASCII 8 или 127)
        else if (c == 8 || c == 127) {
            if (index > 0) {
                index--;
                kprintf("\b \b"); // Сдвигаем курсор назад, стираем пробелом, возвращаем курсор
            }
        }
        // Обычные печатаемые символы
        else {
            // Ограничиваем ввод размером буфера (оставляем 1 байт под '\0')
            if (index < BUFFER_SIZE - 1) {
                buffer[index++] = c;
                uart_putc(c); // Эхо-вывод: отображаем символ пользователю при вводе
            }
        }
    }

    poweroff(); // Недостижимо, и это правильно.
}