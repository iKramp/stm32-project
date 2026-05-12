
#include "register.h"

inline void set_register(volatile uint32_t *reg, uint32_t clear_mask, uint32_t set_value) {
    uint32_t curr = *reg;
    curr &= ~clear_mask;
    curr |= set_value & clear_mask;
    *reg = curr;
}

void reg_read_delay(volatile uint32_t *reg) {
    volatile uint32_t tmp = *reg;
    (void)(tmp);
}

