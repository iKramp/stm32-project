#include "ray_tracer/camdata.hpp"
#include "server.hpp"
#include "client.hpp"
#include "ray_tracer/scene_data.hpp"
#include "ray_tracer/mod.hpp"
extern "C" {
    #include <stdint.h>
    #include "../include/settings.h"
    #include "../include/platform_specific.h"
    #include "../include/draw.h"
    #include "scene_data.h"
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

    // uint8_t *scene_data = get_scene_data();
    //
    // parse_scene_data(scene_data);
    // CamData cam_data = get_cam_data();
    // uint32_t frame = 0;
    //
    // while (1) {
    //     for(int x = 0; x < 480; x++) {
    //         for(int y = 0; y < 272; y++) {
    //             uint32_t color = scramble(x * 272 + y + frame);
    //             draw_pixel(x, y, color | 0xFF000000);
    //         }
    //     }
    //     frame++;
    // }

    if (SERVER) {
        server_main();
    } else {
        client_main();
    }

    while (1) {
        //inf loop
    }
}
