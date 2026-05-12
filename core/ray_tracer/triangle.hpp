#pragma once
#include "hit_record.hpp"
#include "hittable.hpp"
#include "ray.hpp"
#include "stdint.h"

class Triangle {
public:
    uint32_t v0;
    uint32_t v1;
    uint32_t v2;
    uint32_t padding_1;

    uint32_t n0;
    uint32_t n1;
    uint32_t n2;
    uint32_t padding_2;

    uint32_t uv0;
    uint32_t uv1;
    uint32_t uv2;
    uint32_t padding_3;

    Triangle(const uint32_t p0, const uint32_t p1, const uint32_t p2) : v0(p0), v1(p1), v2(p2) {}
    void hit(Ray& r, float t_min, float t_max, HitRecord& hit_record, int obj_index, int tri_index);
    HitData get_hit_data(Ray& r);
};
