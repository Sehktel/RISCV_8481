#ifndef SCHED_H
#define SCHED_H

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


static trap_frame_t* schedule_next(trap_frame_t* current_frame);

static bool task_create(int slot, void (*entry)(void));

trap_frame_t* trap_handler(trap_frame_t* frame);

void run_scheduler_demo(void);
void enter_user_mode(uintptr_t entry, uintptr_t user_sp);
void exit_user_mode_return(void);
void run_user_mode_demo(void);



#endif /* SCHED_H */
