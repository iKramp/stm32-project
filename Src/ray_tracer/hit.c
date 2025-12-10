#include "hit.h"
#include "vec3.h"
#include <math.h>

float triangle_intersect(struct Ray ray, struct Vec3 p0, struct Vec3 p1, struct Vec3 p2, float min_t, float max_t, uint8_t backface_cull) {
    struct Vec3 a = vec3_sub(p1, p0);
    struct Vec3 b = vec3_sub(p2, p0);
    struct Vec3 normal = vec3_normalize(vec3_cross(a, b));
    float d = -(vec3_dot(normal, p0));
    float dot_prod = vec3_dot(normal, ray.direction);

    if (fabsf(dot_prod) < 0.000001) {
        return INFINITY;
    }
    if (backface_cull && dot_prod > 0.0) {
        return INFINITY;
    }

    float t = -(vec3_dot(normal, ray.origin) + d) / vec3_dot(normal, ray.direction);
    if (t < min_t || t > max_t) {
        return INFINITY;
    }

    struct Vec3 hit = vec3_add(ray.origin, vec3_scale(ray.direction, t));
    struct Vec3 c;

    struct Vec3 edge0 = vec3_sub(p1, p0);
    struct Vec3 vp0 = vec3_sub(hit, p0);
    c = vec3_cross(edge0, vp0);
    if (vec3_dot(normal, c) < 0.0) {
        return INFINITY;
    }

    struct Vec3 edge1 = vec3_sub(p2, p1);
    struct Vec3 vp1 = vec3_sub(hit, p1);
    c = vec3_cross(edge1, vp1);
    if (vec3_dot(normal, c) < 0.0) {
        return INFINITY;
    }

    struct Vec3 edge2 = vec3_sub(p0, p2);
    struct Vec3 vp2 = vec3_sub(hit, p2);
    c = vec3_cross(edge2, vp2);
    if (vec3_dot(normal, c) < 0.0) {
        return INFINITY;
    }

    return t;
}
