#include "server.hpp"
#include "client.hpp"
extern "C" {
    #include <stdint.h>
    #include "../include/settings.h"
    #include "../include/platform_specific.h"
    #include "../include/draw.h"
}
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
    platform_init(SERVER);

    draw_rectangle(300, 100, 100, 100, 0xFFFF0000); // Red square for testing

    // panic("Initializing Ethernet...");

    init_packet_handlers(SERVER);

    if (SERVER) {
        server_main();
    } else {
        client_main();
    }

    while (1) {
        //inf loop
    }
}
