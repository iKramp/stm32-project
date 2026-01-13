#pragma once
#include "hit_record.hpp"
#include "hittable.hpp"
#include "ray.hpp"
#include "vec3.hpp"
#include <stdint.h>

enum ChildTriangleMode {
    Children = 0,
    Triangles = 1
};


class BoundingBox {
public:
    Vec3 min;
    uint32_t padding;
    Vec3 max;
    uint32_t padding2;
    BoundingBox() : min(Vec3()), max(Vec3()) {}
    BoundingBox(const Vec3& min, const Vec3& max) : min(min), max(max) {}
    float hit(Ray& r);
    Vec3 center();
};

class alignas(16) BVHNode {
public:
    BoundingBox bounding_box;
    uint32_t left_index;
    uint32_t right_index;
    ChildTriangleMode child_mode;
    void hit(Ray& r, float t_min, float t_max, HitRecord& hit_record, int obj_index);
    HitData get_hit_data(Ray& r, HitRecord& hit_record) {
        return HitData();
    };
};
