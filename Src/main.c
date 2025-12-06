#include "gpio.h"
#include "clock.h"
#include <stdint.h>
#include <string.h>
#include "ltdc.h"
#include "fmc.h"
#include "framebuffer.h"
#include "fb_text.h"
#include "qspi.h"

#define BAD_APPLE_START_ADDR 0x90000000
#define BAD_APPLE_HEIGHT 68
#define BAD_APPLE_WIDTH 120
#define BAD_APPLE_FRAMES 2192

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
    init_qspi();

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
    write_pin('I', 13, 0); // LED on
    wait_ms(500);
    write_pin('I', 13, 1); // LED off

    uint32_t frame = 0;

    addr = (volatile uint32_t *)BAD_APPLE_START_ADDR;
    res = *addr;

    memset(get_fb()->buffer, 0, BAD_APPLE_WIDTH * BAD_APPLE_HEIGHT * 4);

    while (1) {
        uint32_t time = get_time();
        uint8_t *bad_apple_frame = (uint8_t *)(BAD_APPLE_START_ADDR + (frame % BAD_APPLE_FRAMES) * BAD_APPLE_WIDTH * BAD_APPLE_HEIGHT * 4);
        for (int row = 0; row < BAD_APPLE_HEIGHT; row++) {
            uint8_t *src = bad_apple_frame + row * BAD_APPLE_WIDTH * 4;
            volatile uint8_t *dst = get_fb()->buffer + row * get_fb()->width * 4;
            memcpy((void *)dst, (void *)src, BAD_APPLE_WIDTH * 4);
        }
        frame++;
        time = get_time() - time;
        wait_ms(33 - time);
    }
}
