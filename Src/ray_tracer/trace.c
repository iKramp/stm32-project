#include "trace.h"
#include "hit.h"
#include "vec3.h"

struct Vec3 trace_ray(struct Ray ray) {
    ray_normalize(&ray);

    struct Vec3 p0 = {-1, -0.5, 5};
    struct Vec3 p1 = {1, -0.5, 5};
    struct Vec3 p2 = {0, 1, 5};


    if (triangle_intersect(ray, p0, p1, p2, 0.0, 1000000, 0) < 10000000000) {
        struct Vec3 tri_color = {0};
        return tri_color;
    }


    //background stop color
    float factor = ray.direction.y + 0.5;
    factor = factor < 0 ? 0 : (factor > 1 ? 1 : factor);
    struct Vec3 color_top = {1.0, 1.0, 1.0};
    struct Vec3 color_bottom = {0.5, 0.7, 1.0};
    return vec3_add(
        vec3_scale(color_bottom, 1.0 - factor),
        vec3_scale(color_top, factor)
    );
}
