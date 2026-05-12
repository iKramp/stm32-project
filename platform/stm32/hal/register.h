#ifndef REGISTER_H
#define REGISTER_H

#include <stdint.h>
void set_register(volatile uint32_t* reg, uint32_t clear_mask, uint32_t set_value);
void reg_read_delay(volatile uint32_t *reg);

#endif // REGISTER_H
