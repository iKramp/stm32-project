#ifndef HIT_H
#define HIT_H

#include <stdint.h>
#include "ray.hpp"

float triangle_intersect(Ray ray, Vec3 p0, Vec3 p1, Vec3 p2, float min_t, float max_t, uint8_t backface_cull);

#endif // HIT_H
