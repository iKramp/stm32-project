#ifndef COMMON_INTERRUPT_H
#define COMMON_INTERRUPT_H

#include <stdint.h>

uint32_t disable_interrupts();
void restore_interrupts(uint32_t state);

#endif // COMMON_INTERRUPT_H
