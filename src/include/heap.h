#ifndef HEAP_H
#define HEAP_H

#include "types.h"

void kfree(void* ptr);
void heap_init(void);
void heap_dump(void);
void heap_test(void);
void* kmalloc(size_t size);

#endif /* HEAP_H */