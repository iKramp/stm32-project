#ifndef CLOCK_H
#define CLOCK_H

#include "register.h"
#include <stdint.h>

void init_clock();
void wait(volatile uint32_t count);

#endif // CLOCK_H
