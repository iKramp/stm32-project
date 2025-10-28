#ifndef CLOCK_H
#define CLOCK_H

#include "register.h"
#include <stdint.h>

void init_clock();
void wait_micros(volatile uint32_t micros);

#endif // CLOCK_H
