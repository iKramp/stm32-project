#pragma once

#include "vec3.hpp"

class HitData {
public:
    float t;
    Vec3 normal;
    float u;
    float v;
    HitData() : t(0.0f), normal(Vec3()), u(0.0f), v(0.0f) {}
    HitData(float t, const Vec3& normal, float u, float v) : t(t), normal(normal), u(u), v(v) {}
};
