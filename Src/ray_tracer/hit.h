#ifndef HIT_H
#define HIT_H

#include <stdint.h>
#include "ray.h"

float triangle_intersect(struct Ray ray, struct Vec3 p0, struct Vec3 p1, struct Vec3 p2, float min_t, float max_t, uint8_t backface_cull);

#endif // HIT_H
