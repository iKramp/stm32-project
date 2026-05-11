#pragma once

#include <stdint.h>

void tracer_main(volatile uint8_t *framebuffer, uint32_t width, uint32_t height, uint32_t from_x, uint32_t from_y, uint32_t region_width, uint32_t region_height);
uint8_t *get_scene_data();
uint32_t get_scene_data_size();
