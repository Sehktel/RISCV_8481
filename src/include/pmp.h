#ifndef PMP_H
#define PMP_H
#include "kernel_layout.h"
#include "types.h"

// Флаги конфигурации PMP (биты: R=0, W=1, X=2, A[4:3]=01 для TOR)
#define PMP_R (1ULL << 0)
#define PMP_W (1ULL << 1)
#define PMP_X (1ULL << 2)
#define PMP_TOR (1ULL << 3)

void pmp_configure_user(uintptr_t ustack_base, uintptr_t ustack_top);

/* Дефолтный профиль U-задач: один TOR-регион на всё пространство с R+W+X.
 * M-mode не ограничен (нет бита L). Строгие профили (pmptest, per-task в
 * schedule_next) переопределяют его на время своего входа в U-mode. */
void pmp_set_default_user_profile(void);

#endif