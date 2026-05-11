#include "server.hpp"
#include <stdio.h>
extern "C" {
    #include <stdint.h>
    #include <string.h>
    #include "peripherals/clock.h"
    #include "peripherals/ltdc.h"
    #include "peripherals/fmc.h"
    #include "peripherals/eth/ethernet.h"


    #include "rendering/framebuffer.h"
    #include "peripherals/qspi.h"
    #include "rendering/fb_text.h"
    #include "hal/common.h"
    #include "settings.h"
}
#include "ray_tracer/mod.hpp"
#include "net/packet_handler.hpp"

#define QSPI_ADDR 0x90000000

//16MB SDRAM
//128MB QSPI
//128kB program flash
//1MB integrated ram
//
//whole framebuffer takes: 480 * 272 * 4 = 522kB

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

    draw_rectangle(300, 100, 100, 100, 0xFFFF0000); // Red square for testing

    // panic("Initializing Ethernet...");

    init_packet_handlers(SERVER);
    init_ethernet();

    uint8_t *qspi_mem = (uint8_t *)QSPI_ADDR;

    if (SERVER) {
        server_main();
    }

    tracer_main(get_fb()->buffer, get_fb()->width, get_fb()->height, 0, 0, get_fb()->width, get_fb()->height);

    while (1) {
        //inf loop
    }
}
