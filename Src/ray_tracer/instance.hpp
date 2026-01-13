#pragma once
#include "hit_record.hpp"
#include "hittable.hpp"
#include "matrix.hpp"
#include "ray.hpp"

class alignas(16) Instance {
public:
    Affine3 transform;
    int object_index;
    void hit(Ray& r, float t_min, float t_max, HitRecord& hit_record, int obj_index);
    HitData get_hit_data(Ray& r, HitRecord& hit_record) {
        return HitData();
    }
};
