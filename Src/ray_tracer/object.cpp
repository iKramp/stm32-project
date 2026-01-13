#include "object.hpp"
#include "scene_data.hpp"

void Object::hit(Ray& r, float t_min, float t_max, HitRecord& hit_record, int obj_index) {
    SceneData& scene_data = SceneData::get_scene_data();
    scene_data.bvh_buffer[this->bvh_root_index].hit(r, t_min, t_max, hit_record, obj_index);
}
