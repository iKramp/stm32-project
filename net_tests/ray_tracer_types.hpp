#ifndef RTRACER_TYPES_H
#define RTRACER_TYPES_H

#include <cstdint>

class Vec3 {
public:
    float x;
    float y;
    float z;
};

class Mat3 {
    struct Vec3 x_axis;
    uint32_t padding_1;
    struct Vec3 y_axis;
    uint32_t padding_2;
    struct Vec3 z_axis;
    uint32_t padding_3;
};
class Affine3 {
    struct Mat3 matrix;
    struct Vec3 translation;
    uint32_t padding;
};

class CamData {
public:
    uint32_t depth;
    struct Affine3 transform;
    uint32_t canvas_width;
    uint32_t canvas_height;
    uint32_t top_left_x;
    uint32_t top_left_y;
    uint32_t region_width;
    uint32_t region_height;
    float fov;
    uint32_t frame;
    uint32_t random_seed;
};

#endif
