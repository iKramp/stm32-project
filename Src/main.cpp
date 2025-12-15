extern "C" {
    #include <stdint.h>
    #include <string.h>
    #include "peripherals/clock.h"
    #include "peripherals/ltdc.h"
    #include "peripherals/fmc.h"


    #include "rendering/framebuffer.h"
    #include "peripherals/qspi.h"
}
#include "ray_tracer/mod.hpp"

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

    clear_framebuffer(0xFF000000); // Black
    init_display();

    tracer_main(get_fb()->buffer, get_fb()->width, get_fb()->height);

    uint32_t frame = 0;
    while (1) {
        uint32_t time = get_time();
        uint8_t *bad_apple_frame = (uint8_t *)((uint8_t *)BAD_APPLE_START_ADDR + (frame % BAD_APPLE_FRAMES) * BAD_APPLE_WIDTH * BAD_APPLE_HEIGHT * 4);
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
