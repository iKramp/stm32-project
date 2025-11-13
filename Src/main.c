#include "gpio.h"
#include "clock.h"
#include "fmc.c"
#include <stdint.h>

void stop() {
    while (1);
}

int main(void) {
    init_clock();

    init_sdram();

    wait(1000000);

    volatile uint32_t *addr = (uint32_t *)0xD0000000;
    // //do a funny
    *addr = 0xBEEF;
    uint32_t res = *addr;
    if (res != 0xBEEF) {
        stop();
    }


    // PI13 = output (LED)
    set_moder('I', 13, MODER_OUTPUT);

    while (1) {
        write_pin('I', 13, read_pin('I', 13) ^ 1);
        wait(10000000);
    }
}
