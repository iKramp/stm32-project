#include "trace.hpp"
#include "hit.hpp"
#include "vec3.hpp"

Vec3 trace_ray(Ray ray) {
    ray.normalize();

    Vec3 p0 = Vec3(-1, -0.5, 5);
    Vec3 p1 = Vec3(1, -0.5, 5);
    Vec3 p2 = Vec3(0, 1, 5);


    if (triangle_intersect(ray, p0, p1, p2, 0.0, 1000000, 0) < 10000000000) {
        Vec3 tri_color = Vec3();
        return tri_color;
    }


    //background stop color
    float factor = ray.direction.y + 0.5;
    factor = factor < 0 ? 0 : (factor > 1 ? 1 : factor);
    Vec3 color_top = Vec3(1.0, 1.0, 1.0);
    Vec3 color_bottom = Vec3(0.5, 0.7, 1.0);
    return (color_bottom * (1.0 - factor)) + (color_top * factor);
}
