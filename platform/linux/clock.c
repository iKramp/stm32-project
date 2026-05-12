#include "../../include/clock.h"
#include <time.h>

uint32_t get_time() {
    //miliseconds from linux, modulo 2^32
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000) % 0x100000000;
}
