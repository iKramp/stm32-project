#pragma once
#include "vec3.hpp"
#include <stdint.h>

class Vertex {
public:
    Vec3 position;
    uint32_t padding_1;
    Vertex() : position(0, 0, 0) {}
    Vertex(const Vec3& pos) : position(pos) {}
};
