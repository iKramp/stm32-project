#include "hit_record.hpp"
#include "hittable.hpp"
#include "scene_data.hpp"
#include "vec3.hpp"
#include "math_wrapper.hpp"

void Triangle::hit(Ray& ray, float min_t, float max_t, HitRecord& hit_record, int obj_index, int tri_index) {
    SceneData& scene_data = SceneData::get_scene_data();
    Vec3 p0 = scene_data.vert_buffer[this->v0].position;
    Vec3 p1 = scene_data.vert_buffer[this->v1].position;
    Vec3 p2 = scene_data.vert_buffer[this->v2].position;

    Vec3 a = p1 - p0;
    Vec3 b = p2 - p0;
    Vec3 normal = a.cross(b).normalize();
    float d = -(normal.dot(p0));
    float dot_prod = normal.dot(ray.direction);

    if (fabsf(dot_prod) < 0.000001) {
        return;
    }
    if (dot_prod > 0.0) {
        return;
    }

    float t = -(normal.dot(ray.origin) + d) / normal.dot(ray.direction);
    if (t < min_t || t > max_t) {
        return;
    }

    Vec3 hit = ray.origin + ray.direction * t;
    Vec3 c;

    Vec3 edge0 = p1 - p0;
    Vec3 vp0 = hit - p0;
    c = edge0.cross(vp0);
    if (normal.dot(c) < 0.0) {
        return;
    }

    Vec3 edge1 = p2 - p1;
    Vec3 vp1 = hit - p1;
    c = edge1.cross(vp1);
    if (normal.dot(c) < 0.0) {
        return;
    }

    Vec3 edge2 = p0 - p2;
    Vec3 vp2 = hit - p2;
    c = edge2.cross(vp2);
    if (normal.dot(c) < 0.0) {
        return;
    }

    uint8_t buffer[64];
    snprintf((char *)buffer, 64, "testing %d   ", tri_index);
    set_cursor_position(1, 0);
    write_text((char *)buffer);

    hit_record.try_add_hit(t, obj_index, tri_index);
}
