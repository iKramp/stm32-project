#include "ray_tracer/camdata.hpp"
#include "server.hpp"
#include "client.hpp"
#include "ray_tracer/scene_data.hpp"
#include "ray_tracer/mod.hpp"
#include <stdio.h>
extern "C" {
    #include <stdint.h>
    #include "../include/settings.h"
    #include "../include/platform_specific.h"
    #include "../include/draw.h"
    #include "../include/clock.h"
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

    init_packet_handlers(SERVER);

    // uint32_t now = get_time();
    // uint32_t state = 0xDEADBEEF;
    // for (int i = 0; i < 1000; i++) {
    //     printf("%d", state);
    //     state = scramble(state);
    // }
    // uint32_t elapsed = get_time() - now;
    // printf("Scrambled 1000 numbers in %u ms\n", elapsed);

    // uint8_t *scene_data = get_scene_data();
    // uint32_t SCALING_FACTOR = 20;
    //
    // parse_scene_data(scene_data);
    // CamData cam_data = get_cam_data();
    // cam_data.canvas_height = 272 / SCALING_FACTOR;
    // cam_data.canvas_width = 480 / SCALING_FACTOR;
    //
    // uint32_t frame = 0;
    //
    //
    // while (1) {
    //     for(int x = 0; x < 480 / SCALING_FACTOR; x++) {
    //         for(int y = 0; y < 272 / SCALING_FACTOR; y++) {
    //             uint32_t color = tracer_main(cam_data, x, y);
    //             // uint32_t color = scramble(x * 73856093 ^ y * 19349663 ^ frame * 83492791);
    //             // draw_pixel(x, y, color | 0xFF000000);
    //             draw_scaled_pixel(x, y, color | 0xFF000000, SCALING_FACTOR);
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
