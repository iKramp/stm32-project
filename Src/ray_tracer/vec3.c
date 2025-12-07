#include "vec3.h"

struct Vec3 vec3_add(struct Vec3 a, struct Vec3 b) {
    struct Vec3 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

struct Vec3 vec3_sub(struct Vec3 a, struct Vec3 b) {
    struct Vec3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

struct Vec3 vec3_scale(struct Vec3 v, float s) {
    struct Vec3 result;
    result.x = v.x * s;
    result.y = v.y * s;
    result.z = v.z * s;
    return result;
}

float vec3_dot(struct Vec3 a, struct Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float vec3_length(struct Vec3 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

float vec3_length2(struct Vec3 v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

struct Vec3 vec3_normalize(struct Vec3 v) {
    float len = vec3_length(v);
    if (len == 0) {
        return v; // Avoid division by zero
    }
    return vec3_scale(v, 1.0f / len);
}
