#pragma once
#include "hit_record.hpp"
#include "hittable.hpp"
#include "ray.hpp"

class Object {
public:
    int bvh_root_index;
    int normal_image_index;
    int texture_image_index;
    
    void hit(Ray& r, float t_min, float t_max, HitRecord& hit_record, int obj_index);
    HitData get_hit_data(Ray& r, HitRecord& hit_record) {
        return HitData();
    }
};
