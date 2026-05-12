#include "../../../include/draw.h"
#include "../rendering/framebuffer.h"


void draw_scaled_pixel(uint32_t x, uint32_t y, uint32_t color, uint32_t scaling_factor) {
    struct FrameBuffer *fb = get_fb();

    for (int dx = 0; dx < scaling_factor; dx++) {
        for (int dy = 0; dy < scaling_factor; dy++) {
            int draw_x = x * scaling_factor + dx;
            int draw_y = y * scaling_factor + dy;
            if (draw_x < fb->width && draw_y < fb->height) {
                int index = (draw_y * fb->width + draw_x) * 4;
                *((volatile uint32_t *)(fb->buffer + index)) = color;
            }
        }
    }
}

void get_fb_dimensions(uint32_t *width, uint32_t *height) {
    struct FrameBuffer *fb = get_fb();
    *width = fb->width;
    *height = fb->height;
}
