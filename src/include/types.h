#ifndef TYPES_H
#define TYPES_H

/* ============================================================================
 * БАЗОВЫЕ ТИПЫ (RV64 / LP64 Bare-Metal)
 * ============================================================================ */
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long int64_t;
typedef unsigned long uint64_t;

typedef unsigned long size_t;
typedef signed long ssize_t;
typedef unsigned long uintptr_t;
typedef signed long intptr_t;

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef _Bool bool;
#define true 1
#define false 0

/* Выравнивание вверх до границы (x должен быть беззнаковым) */
#define ALIGN_UP(x, a) (((x) + (a) - 1) & ~((a) - 1))
#define ALIGN16(x) ALIGN_UP(x, 16)

#endif /* TYPES_H */
