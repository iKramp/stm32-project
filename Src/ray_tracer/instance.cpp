#include "instance.hpp"
#include "scene_data.hpp"


void Instance::hit(Ray& r, float t_min, float t_max, HitRecord& hit_record, int obj_index) {
    SceneData& scene_data = SceneData::get_scene_data();
    Affine3 inverse = this->transform.inverse();
    Ray ray = Ray(inverse.mul_point(r.origin), inverse * r.direction);
    scene_data.obj_buffer[this->object_index].hit(ray, t_min, t_max, hit_record, obj_index);
}
