#ifndef RAY_H
#define RAY_H

#include <stdint.h>
#include <math.h>
#include "vec3.h"

struct Ray {
    struct Vec3 origin;
    struct Vec3 direction;
};

void ray_normalize(struct Ray *ray);

#endif // RAY_H
