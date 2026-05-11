#ifndef NVIC_H
#define NVIC_H

#include <stdint.h>
#define NVIC_REG 0xE000E100

#define NVIC_ETH 61

void enable_irq(uint64_t irq_num);
void disable_irq(uint64_t irq_num);
uint32_t irq_save(void);
void irq_restore(uint32_t primask);

#endif // NVIC_H
