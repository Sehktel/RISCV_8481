#ifndef POWEROFF_H
#define POWEROFF_H

// Регистры управления питанием QEMU (SiFive Test Finisher)
#define SYSCON_TEST_ADDR 0x100000
#define FINISHER_PASS 0x5555

void poweroff(void);

#endif /* POWEROFF_H */