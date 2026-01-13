#include "trace.hpp"
#include "bvh.hpp"
#include "hit_record.hpp"
#include "instance.hpp"
#include "scene_data.hpp"
#include "triangle.hpp"
#include "vec3.hpp"
#include <stdio.h>

extern "C" {
    #include "../rendering/fb_text.h"
}

NormalUv get_normal_uv(
        Vec3 hit, 
        Vec3 vert0,
        Vec3 vert1,
        Vec3 vert2,
        Vec3 n0, 
        Vec3 n1,
        Vec3 n2,
        UV uv0,
        UV uv1,
        UV uv2
) {
    Vec3 v0_pos = vert1 - vert0;
    Vec3 v1_pos = vert2 - vert0;
    Vec3 v2_pos = hit - vert0;

    float d00 = v0_pos.dot(v0_pos);
    float d01 = v0_pos.dot(v1_pos);
    float d11 = v1_pos.dot(v1_pos);
    float d20 = v2_pos.dot(v0_pos);
    float d21 = v2_pos.dot(v2_pos);

    float denom = d00 * d11 - d01 * d01;
    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0 - v - w;

    Vec3 normal = (n0 * u + n1 * v + n2 * w).normalize();
    float u_ = uv0.u * u + uv1.u * v + uv2.u * w;
    float v_ = uv0.v * u + uv1.v * v + uv2.v * w;

    NormalUv result;
    result.normal = normal;
    result.uv = UV(u_, v_);
    return result;
}



Vec3 trace_ray(Ray ray) {
    ray.normalize();
    // ray.origin = ray.origin + Vec3(0, 0, -10);
    HitRecord hit_record = HitRecord();
    SceneData& scene_data = SceneData::get_scene_data();
    
    for (int i = 0; i < scene_data.instance_count; i++) {
        scene_data.instance_buffer[i].hit(ray, 0.0, 10000000, hit_record, i);
    }

    if (hit_record.obj_index != -1) {
        set_cursor_position(0, 0);
        uint8_t int_buffer[64];
        snprintf((char *)int_buffer, 64, "Hit tri %d", hit_record.sub_index);
        write_text((char *)int_buffer);



        Triangle tri = scene_data.triangle_buffer[hit_record.sub_index];
        Affine3 transform = scene_data.instance_buffer[hit_record.obj_index].transform;

        Vec3 v0 = scene_data.vert_buffer[tri.v0].position;
        Vec3 v1 = scene_data.vert_buffer[tri.v1].position;
        Vec3 v2 = scene_data.vert_buffer[tri.v2].position;
        v0 = transform.mul_point(v0);
        v1 = transform.mul_point(v1);
        v2 = transform.mul_point(v2);

        Vec3 n0;
        Vec3 n1;
        Vec3 n2;
        if (tri.n0 != (uint32_t)-1) {
            //saved normals
            n0 = transform * scene_data.normal_buffer[tri.n0];
            n1 = transform * scene_data.normal_buffer[tri.n1];
            n2 = transform * scene_data.normal_buffer[tri.n2];
        } else {
            //calculate normals
            Vec3 edge1 = v1 - v0;
            Vec3 edge2 = v2 - v0;
            Vec3 normal = edge1.cross(edge2).normalize();
            n0 = normal;
            n1 = normal;
            n2 = normal;
        }

        UV uv0;
        UV uv1;
        UV uv2;
        if (tri.uv0 != (uint32_t)-1) {
            uv0 = scene_data.uv_buffer[tri.uv0];
            uv1 = scene_data.uv_buffer[tri.uv1];
            uv2 = scene_data.uv_buffer[tri.uv2];
        } else {
            uv0 = UV(0, 0);
            uv1 = UV(0, 0);
            uv2 = UV(0, 0);
        }

        Vec3 hit_point = ray.origin + ray.direction * hit_record.t;
        NormalUv normal_uv = get_normal_uv(
            hit_point,
            v0, v1, v2,
            n0, n1, n2,
            uv0, uv1, uv2
        );

        //normal into color
        Vec3 normal = normal_uv.normal;
        return (normal + Vec3(1.0, 1.0, 1.0)) * 0.5;
    }

    //background stop color
    float factor = ray.direction.y + 0.5;
    factor = factor < 0 ? 0 : (factor > 1 ? 1 : factor);
    Vec3 color_top = Vec3(1.0, 1.0, 1.0);
    Vec3 color_bottom = Vec3(0.5, 0.7, 1.0);
    return (color_bottom * (1.0 - factor)) + (color_top * factor);
}
