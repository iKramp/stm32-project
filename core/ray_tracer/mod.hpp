#pragma once

#include <stdint.h>
#include "camdata.hpp"

void parse_scene_data(uint8_t *raw_data);

uint32_t tracer_main(
    CamData &cam_data,
    uint32_t x,
    uint32_t y
);
