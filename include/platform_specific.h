#ifndef PLATFORM_SPECIFIC_H
#define PLATFORM_SPECIFIC_H

#include <stdint.h>

void platform_init(uint8_t server);
uint8_t *get_data_buffer();

#endif // PLATFORM_SPECIFIC_H
