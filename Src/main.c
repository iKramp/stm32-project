#include "gpio.h"
#include "clock.h"
#include <stdint.h>
#include "ltdc.h"
#include "fmc.h"
#include "framebuffer.h"
#include "fb_text.h"

void stop() {
    while (1);
}

static inline uint32_t scramble(uint32_t x)
{
    x ^= x >> 17;
    x *= 0xed5ad4bbU;
    x ^= x >> 11;
    x *= 0xac4c1b51U;
    x ^= x >> 15;
    x *= 0x31848babU;
    x ^= x >> 14;
    return x;
}

int main(void) {
    init_clock();

    init_sdram();

    wait_ms(1);

    volatile uint32_t *addr = (uint32_t *)0xD0000000;
    // //do a funny
    *addr = 0xBEEF;
    uint32_t res = *addr;
    if (res != 0xBEEF) {
        stop();
    }

    for (int i = 0; i < (480*272); i++) {
        addr[i] = scramble(i);
    }

    init_display();

    draw_rectangle(10, 10, 100, 100, 0xFF00FF00); // Draw a green square

    // PI13 = output (LED)
    set_moder('I', 13, MODER_OUTPUT);

#define BUF_LEN 61
    uint32_t counter = 0;
    uint8_t buffer[BUF_LEN];

    while (1) {
        write_pin('I', 13, read_pin('I', 13) ^ 1);
        wait_ms(1000);
    
        for (int i = 0; i < BUF_LEN; i++) {
            uint8_t chr = scramble(counter << 5 | i);
            buffer[i] = (chr % (0x7f - 0x20)) + 0x20;
        }
        buffer[BUF_LEN - 1] = '\0';

        write_text((char *)buffer);
        counter++;
    }
}
