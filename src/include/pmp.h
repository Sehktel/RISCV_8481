#ifndef PMP_H
#define PMP_H
#include "types.h"
#include "kernel_layout.h"


void pmp_configure_user(uintptr_t ustack_base, uintptr_t ustack_top);


#endif