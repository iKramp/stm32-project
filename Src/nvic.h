#ifndef NVIC_H
#define NVIC_H

#include <stdint.h>
#define NVIC_REG 0xE000E100

void enable_irq(uint64_t irq_num);
void disable_irq(uint64_t irq_num);

#endif // NVIC_H
