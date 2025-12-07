#include "framebuffer.h"

static struct FrameBuffer fb = {
    .buffer = (volatile uint8_t *)0xD0000000, // Example framebuffer address in SDRAM
    .width = 480,
    .height = 272,
    .bpp = 4, // bytes per pixel (assuming ARGB8888)
};

void init_framebuffer(volatile uint8_t *address, uint32_t width, uint32_t height) {
    fb.buffer = address;
    fb.width = width;
    fb.height = height;
}

void draw_pixel(int x, int y, uint32_t color) {
    int offset = y * fb.width * fb.bpp + x * fb.bpp;
    *(volatile uint32_t *)(fb.buffer + offset) = color;
}

void clear_framebuffer(uint32_t color) {
    draw_rectangle(0, 0, fb.width, fb.height, color);
}

void draw_rectangle(int x, int y, int width, int height, uint32_t color) {
    x = x > 0 ? x : 0;
    y = y > 0 ? y : 0;

    int x_end = (x + width) < fb.width ? (x + width) : fb.width;
    int y_end = (y + height) < fb.height ? (y + height) : fb.height;
    for (int j = y; j < y_end; j++) {
        for (int i = x; i < x_end; i++) {
            draw_pixel(i, j, color);
        }
    }
}

struct FrameBuffer* get_fb() {
    return &fb;
}
