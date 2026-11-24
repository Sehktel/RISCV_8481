#include "../include/sched.h"
#include "../include/console.h" /* kprintf */
#include "../include/csr.h"
#include "../include/csr.h"  /* если в sched.c есть CSR-обёртки */
#include "../include/heap.h" /* kmalloc, kfree */
#include "../include/kernel_layout.h"
#include "../include/pmp.h"
#include "../include/timer.h" /* см. пункт 3 */
#include "../include/types.h"
#include "../include/uart.h" /* print */

static task_t tasks[MAX_TASKS];
// int current_task_idx = -1;
// int scheduler_active = 0;
// int timer_ticks_count = 0;
// uintptr_t shell_return_sp = 0;

static int current_task_idx = -1;
static int scheduler_active = 0;
static int timer_ticks_count = 0;
static uintptr_t shell_return_sp = 0;

void sched_set_shell_return_sp(uint64_t sp) {
    shell_return_sp = sp;
}
uint64_t sched_get_shell_return_sp(void) {
    return shell_return_sp;
}

/* Готовит mstatus к mret в нужный режим: 0 = U-mode (MPP=00), 1 = M-mode (MPP=11). */
static void set_return_mode(int machine_mode) {
    uint64_t m = read_csr(mstatus);
    m &= ~(3ULL << 11); /* стереть MPP */
    if (machine_mode) m |= (3ULL << 11);
    write_csr(mstatus, m);
}

/* Корректно снимает задачу с исполнения: слот покидает очередь,
 * таймер и планировщик гасятся, управление уходит в M-mode Shell. */
static void kill_current_task(trap_frame_t* frame) {
    if (current_task_idx >= 0) tasks[current_task_idx].state = TASK_EXITED;
    current_task_idx = -1;
    scheduler_active = 0;
    timer_enable(false);

    frame->a0 = (uint64_t)-1;
    frame->sp = shell_return_sp; /* НЕ frame + sizeof(trap_frame_t) */
    // frame->sp = (uintptr_t)frame + sizeof(trap_frame_t);
    write_csr(mepc, (uint64_t)exit_user_mode_return);
    set_return_mode(1);
}
/**
 * @brief Выбирает следующую готовую задачу по кругу (Round-Robin).
 */
trap_frame_t* schedule_next(trap_frame_t* current_frame) {
    /* Сохраняем контекст прерванной задачи (если она ещё жива) */
    if (current_task_idx >= 0 && tasks[current_task_idx].state == TASK_RUNNING) {
        tasks[current_task_idx].mepc = read_csr(mepc);
        tasks[current_task_idx].frame = current_frame;
        tasks[current_task_idx].state = TASK_READY;
    }

    for (int offset = 1; offset <= MAX_TASKS; offset++) {
        int next_idx = (current_task_idx + offset) % MAX_TASKS;
        if (tasks[next_idx].state != TASK_READY) continue; /* мёртвых и свободных не трогаем — ни PMP, ни mepc */

        current_task_idx = next_idx;
        tasks[next_idx].state = TASK_RUNNING;

        /* Профиль защиты — только для выбранной задачи */
        // pmp_configure_user((uintptr_t)tasks[next_idx].ustack, (uintptr_t)(tasks[next_idx].ustack + 4096));

        write_csr(mepc, tasks[next_idx].mepc);
        set_return_mode(0); /* mret ведёт в U-задачу — назначено явно */
        return tasks[next_idx].frame;
    }

    /* Готовых задач нет — корректно сворачиваем демонстрацию */
    timer_enable(false);
    scheduler_active = 0;
    current_task_idx = -1;
    current_frame->sp = shell_return_sp;
    write_csr(mepc, (uint64_t)exit_user_mode_return);
    set_return_mode(1);
    return current_frame;
}
// trap_frame_t* schedule_next(trap_frame_t* current_frame) {
//     if (current_task_idx >= 0 && tasks[current_task_idx].state == TASK_RUNNING) {
//         tasks[current_task_idx].mepc = read_csr(mepc);
//         tasks[current_task_idx].frame = current_frame;
//         tasks[current_task_idx].state = TASK_READY;
//     }

//     for (int offset = 1; offset <= MAX_TASKS; offset++) {
//         int next_idx = (current_task_idx + offset) % MAX_TASKS;
//         pmp_configure_user((uintptr_t)tasks[next_idx].ustack, (uintptr_t)(tasks[next_idx].ustack + 4096));
//         if (tasks[next_idx].state == TASK_READY) {
//             current_task_idx = next_idx;
//             tasks[next_idx].state = TASK_RUNNING;
//             write_csr(mepc, tasks[next_idx].mepc);
//             return tasks[next_idx].frame;
//         }
//     }
//     return current_frame;
// }

trap_frame_t* trap_handler(trap_frame_t* frame) {
    uint64_t mcause = read_csr(mcause);
    uint64_t mepc = read_csr(mepc);
    uint64_t mtval = read_csr(mtval);

    // 1. Обработка прерывания машинного таймера CLINT (Вытесняющий Round-Robin)
    if (mcause == MCAUSE_TIMER_INTERRUPT) {
        timer_set_next(SCHED_QUANTUM_TICKS);

        if (!scheduler_active) return frame;

        /* Железо сохраняет в MPP режим, из которого пришло прерывание:
         * MPP=0 — тик застал U-задачу, её контекст можно переключать.
         * MPP=M — тик застал M-mode код (enter_user_mode, обработчик).
         * Сохранять такой контекст в таблицу задач нельзя: он не принадлежит
         * ни одной задаче. Перевооружились — и вернулись куда пришли. */
        uint64_t mpp = (read_csr(mstatus) >> 11) & 3;
        if (mpp != 0) {
            return frame;
        }

        timer_ticks_count++;
        kprintf("[TIMER IRQ #%d] ...\n");

        // timer_set_next(SCHED_QUANTUM_TICKS);

        // if (scheduler_active) {
        //     timer_ticks_count++;
        //     kprintf("[TIMER IRQ #%d] Переключение контекста (Round-Robin)...\n", timer_ticks_count);

        // После 6 квантов (600 мс) завершаем демонстрацию и возвращаемся в Shell
        if (timer_ticks_count >= 6) {
            timer_enable(false);
            scheduler_active = 0;
            frame->sp = shell_return_sp;
            write_csr(mepc, (uint64_t)exit_user_mode_return);
            set_return_mode(1); /* MPP=M: mret ведёт в M-mode Shell */
            return frame;
        }

        trap_frame_t* next = schedule_next(frame);

        /* Планировщик назначает режим возврата явно: тик мог прийти,
         * пока CPU был в M-mode (в трапе или в kprintf) — тогда без
         * этого mret продолжит задачу в M-режиме, и её первый ecall
         * даст mcause=11 вместо 8. */
        set_return_mode(0); /* MPP=U: mret ведёт в U-задачу */

        return next;
        // }
        // return frame;
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
            kill_current_task(frame);
            return frame;
        }
        // case SYS_EXIT: {
        //     uint64_t exit_code = frame->a0;
        //     kprintf("[KERNEL] Процесс U-mode завершился через SYS_EXIT (код: %lu)\n", exit_code);
        //     frame->sp = (uintptr_t)frame + sizeof(trap_frame_t);
        //     write_csr(mepc, (uint64_t)exit_user_mode_return);
        //     uint64_t mstatus = read_csr(mstatus);
        //     mstatus |= MSTATUS_MPP_MASK;
        //     write_csr(mstatus, mstatus);
        //     return frame;
        // }
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
        kill_current_task(frame);
        return frame;
        // frame->a0 = (uint64_t)-1;
        // frame->sp = (uintptr_t)frame + sizeof(trap_frame_t);
        // write_csr(mepc, (uint64_t)exit_user_mode_return);
        // uint64_t mstatus = read_csr(mstatus);
        // mstatus |= MSTATUS_MPP_MASK;
        // write_csr(mstatus, mstatus);
        // return frame;
    }

    kprintf("\n[PANIC] Необработанное исключение: mcause=%lu, mepc=0x%lx, mtval=0x%lx\n", mcause, mepc, mtval);
    for (;;) {
        asm volatile("wfi");
    }
}

/**
 * @brief Обёртка пользовательского пространства над инструкцией ecall.
 * Выполняется в U-mode: прямая запись в UART0 здесь вызвала бы Load/Store Fault
 * при включённом PMP, поэтому обращение к ядру идёт строго через ABI ecall.
 */
uint64_t sys_call(uint64_t sys_num, uint64_t arg0, uint64_t arg1) {
    register uint64_t a0 asm("a0") = arg0;
    register uint64_t a1 asm("a1") = arg1;
    register uint64_t a7 asm("a7") = sys_num;

    asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a7) : "memory");
    return a0;
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

void run_scheduler_demo(void) {
    for (int i = 0; i < MAX_TASKS; i++) {
        tasks[i].state = TASK_UNUSED;
        tasks[i].kstack = NULL;
        tasks[i].ustack = NULL;
        tasks[i].frame = NULL;
    }
    //    for (int i = 0; i < MAX_TASKS; i++) tasks[i].state = TASK_UNUSED;
    current_task_idx = -1;
    timer_ticks_count = 0;
    pmp_set_default_user_profile();

    if (!task_create(0, worker_task_a) || !task_create(1, worker_task_b)) {
        kprintf("[ERROR] Не удалось выделить память под задачи планировщика\n");
        for (int i = 0; i < MAX_TASKS; i++) {
            kfree(tasks[i].kstack);
            kfree(tasks[i].ustack);
            tasks[i].kstack = NULL;
            tasks[i].ustack = NULL;
            tasks[i].frame = NULL;
        }
        return;
    }

    // Этап 10: сужаем границы
    // pmp_configure_user((uintptr_t)tasks[0].ustack, tasks[0].frame->sp);

    // write_csr(pmpaddr0, (~0ULL) >> 10);
    // write_csr(pmpcfg0, 0x0FULL);

    current_task_idx = 0;
    tasks[0].state = TASK_RUNNING;
    timer_ticks_count = 0;
    scheduler_active = 1;

    // Запоминаем вершину стека ядра для возврата в Shell после завершения демо
    // uint64_t cur_sp;
    // asm volatile("mv %0, sp" : "=r"(cur_sp));
    // shell_return_sp = cur_sp - 112; // Учитываем фрейм enter_user_mode
    uint64_t cur_sp;
    asm volatile("mv %0, sp" : "=r"(cur_sp));
    shell_return_sp = cur_sp - 112; /* фрейм enter_user_mode */

    // pmp_configure_user(...);          /* строгий профиль */
    // enter_user_mode((uintptr_t)rogue_uart_task, user_sp);

    kprintf("[SCHED] Запуск вытесняющего планировщика Round-Robin (2 потока U-mode, квант 100 мс)...\n");
    timer_enable(true);

    enter_user_mode(tasks[0].mepc, tasks[0].frame->sp);

    kprintf("[SCHED] Демонстрация планировщика завершена. Освобождаем память задач...\n");
    for (int i = 0; i < 2; i++) {
        kfree(tasks[i].kstack);
        kfree(tasks[i].ustack);
    }
}

/**
 * @brief Инициализирует слот задачи в планировщике и формирует начальный trap_frame_t.
 */
bool task_create(int slot, void (*entry)(void)) {
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

/* Сообщает планировщику, что текущая задача отсутствует: следующий
 * kill_current_task не пометит никакой слот как завершённый.
 * Используется демо, которые входят в U-mode вне задач планировщика. */
void sched_clear_current_task(void) {
    current_task_idx = -1;
}
