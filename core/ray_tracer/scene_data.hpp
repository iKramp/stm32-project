#pragma once

#include "bvh.hpp"
#include "hittable.hpp"
#include "instance.hpp"
#include "object.hpp"
#include "triangle.hpp"
#include "uv.hpp"
#include "vec3.hpp"
#include "vertex.hpp"
#include <stdint.h>

class SceneData {
public:
    Vertex *vert_buffer;
    Vec3 *normal_buffer;
    UV *uv_buffer;
    Triangle *triangle_buffer;
    BVHNode *bvh_buffer;
    Object *obj_buffer;
    Instance *instance_buffer;
    uint32_t instance_count;

    static SceneData& get_scene_data();
    static void set_scene_data(SceneData& scene_data);
};
