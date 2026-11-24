#ifndef CONSOLE_H
#define CONSOLE_H

#include "types.h"

void print_number(uint64_t num, int base, int min_width);
void kprintf(const char* fmt, ...);

#endif