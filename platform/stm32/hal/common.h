#ifndef COMMON_H
#define COMMON_H

#include "../rendering/fb_text.h"

#define align_addr(addr, alignment) (((addr) + (alignment - 1)) & ~(alignment - 1))

void panic(const char *message);
uint32_t get_uid();
void print_cache_info();
void enable_caches();

#endif // COMMON_H
