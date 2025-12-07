#include "nvic.h"

void enable_irq(uint64_t irq_num) {
    volatile uint32_t *NVIC_ISER = (uint32_t *)(NVIC_REG + (irq_num / 32) * 4);
    *NVIC_ISER = (1 << (irq_num % 32));
}

void disable_irq(uint64_t irq_num) {
    volatile uint32_t *NVIC_ICER = (uint32_t *)(NVIC_REG + 0x80 + (irq_num / 32) * 4);
    *NVIC_ICER = (1 << (irq_num % 32));
}
