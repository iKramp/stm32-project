#include "fpu.h"
#include <stdint.h>

volatile uint32_t *CPACR = (uint32_t *)0xE000ED88;

void init_fpu() {
    uint32_t cpacr = *CPACR;
    cpacr |= (0b1111 << 20);
    *CPACR = cpacr;

    __asm volatile ("dsb"); //data sync barrier
    __asm volatile ("isb"); //instruction sync barrier
}
