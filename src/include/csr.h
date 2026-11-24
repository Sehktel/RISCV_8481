#ifndef CSR_H
#define CSR_H

/* Обёртки над CSR-инструкциями RISC-V.
 * Имена регистров (pmpaddr0, mie, mscratch...) — НЕ C-идентификаторы,
 * а лексемы для ассемблера: #reg склеивает их в строку инструкции,
 * поэтому они не должны объявляться как переменные. */

#define write_csr(reg, val) ({ asm volatile("csrw " #reg ", %0" ::"r"(val)); })
#define read_csr(reg)                                                                                                  \
    ({                                                                                                                 \
        uint64_t __tmp;                                                                                                \
        asm volatile("csrr %0, " #reg : "=r"(__tmp));                                                                  \
        __tmp;                                                                                                         \
    })
#define set_csr(reg, bit) ({ asm volatile("csrs " #reg ", %0" ::"r"(bit)); })
#define clear_csr(reg, bit) ({ asm volatile("csrc " #reg ", %0" ::"r"(bit)); })

#endif /* CSR_H */
