#include "bvh.hpp"
#include "math_wrapper.hpp"
#include "scene_data.hpp"

float BoundingBox::hit(Ray& r) {
    Vec3 t_min = (this->min - r.origin) / r.direction;
    Vec3 t_max = (this->max - r.origin) / r.direction;

    if (t_min.x > t_max.x) {
        float tmp = t_min.x;
        t_min.x = t_max.x;
        t_max.x = tmp;
    }
    if (t_min.y > t_max.y) {
        float tmp = t_min.y;
        t_min.y = t_max.y;
        t_max.y = tmp;
    }
    if (t_min.z > t_max.z) {
        float tmp = t_min.z;
        t_min.z = t_max.z;
        t_max.z = tmp;
    }

    float t_near = t_min.x > t_min.y && t_min.x > t_min.z ? t_min.x : (t_min.y > t_min.z ? t_min.y : t_min.z);
    float t_far =  t_max.x > t_max.y && t_max.x > t_max.z ? t_max.x : (t_max.y > t_max.z ? t_max.y : t_max.z);

    if (t_near < INFINITY && t_near < t_far && t_far > 0) {
        return t_near;
    }
    return INFINITY;
}

Vec3 BoundingBox::center() {
    return (this->min + this->max) * 0.5;
}

void BVHNode::hit(Ray& r, float t_min, float t_max, HitRecord& hit_record, int obj_index) {
    SceneData& scene_data = SceneData::get_scene_data();
    t_max = t_max < hit_record.t ? t_max : hit_record.t;
    float t_hit = bounding_box.hit(r);
    if (t_hit < 0 || t_hit < t_min || t_hit > t_max) {
        return;
    }

    if (this->child_mode == ChildTriangleMode::Children) {
        BVHNode& first_node = scene_data.bvh_buffer[this->left_index];
        BVHNode& second_node = scene_data.bvh_buffer[this->right_index];
        Vec3 first_center = first_node.bounding_box.center();
        Vec3 second_center = second_node.bounding_box.center();
        float dist_1 = (first_center - r.origin).length2();
        float dist_2 = (second_center - r.origin).length2();
        if (dist_1 < dist_2) {
            first_node.hit(r, t_min, t_max, hit_record, obj_index);
            second_node.hit(r, t_min, t_max, hit_record, obj_index);
        } else {
            second_node.hit(r, t_min, t_max, hit_record, obj_index);
            first_node.hit(r, t_min, t_max, hit_record, obj_index);
        }
    } else {
        uint32_t first_tri = this->left_index;
        uint32_t last_tri = this->right_index;
        for (int i = first_tri; i <= last_tri; i++) {
            scene_data.triangle_buffer[i].hit(r, t_min, t_max, hit_record, obj_index, i);
        }
    }
}
