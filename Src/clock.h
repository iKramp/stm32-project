#ifndef CLOCK_H
#define CLOCK_H

#include "register.h"
#include <stdint.h>

#define SYS_CLK_HSI 0b000
#define SYS_CLK_CSI 0b001
#define SYS_CLK_HSE 0b010
#define SYS_CLK_PLL1 0b011
#define RCC_REG 0x58024400
#define RCC_TIM6_REG 0x40001000
#define SYSTICK_REG 0xE000E010

void init_clock();
void wait(volatile uint32_t count);
void wait_ms(uint32_t ms);
uint32_t get_time();

#endif // CLOCK_H
