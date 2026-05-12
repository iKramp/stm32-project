#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>

struct FrameBuffer {
    volatile uint8_t *buffer;
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
};

struct FrameBuffer* get_fb();
void init_framebuffer(volatile uint8_t *address, uint32_t width, uint32_t height);
void clear_framebuffer(uint32_t color);



#endif // FRAMEBUFFER_H
