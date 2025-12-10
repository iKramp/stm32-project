#ifndef VEC3_H
#define VEC3_H

#include <math.h>

struct Vec3 {
    float x;
    float y;
    float z;
};

struct Vec3 vec3_add(struct Vec3 a, struct Vec3 b);
struct Vec3 vec3_sub(struct Vec3 a, struct Vec3 b);
struct Vec3 vec3_scale(struct Vec3 v, float s);

float vec3_dot(struct Vec3 a, struct Vec3 b);
struct Vec3 vec3_cross(struct Vec3 a, struct Vec3 b);

float vec3_length(struct Vec3 v);
float vec3_length2(struct Vec3 v);
struct Vec3 vec3_normalize(struct Vec3 v);

#endif // VEC3_H
