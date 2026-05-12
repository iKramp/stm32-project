#ifndef COMMON_DRAW_H
#define COMMON_DRAW_H

#include <stdint.h>

void draw_scaled_pixel(uint32_t x, uint32_t y, uint32_t color, uint32_t scaling_factor);
void draw_rectangle(int x, int y, int width, int height, uint32_t color);
void draw_pixel(int x, int y, uint32_t color);
void get_fb_dimensions(uint32_t *width, uint32_t *height);

#endif // COMMON_DRAW_H
