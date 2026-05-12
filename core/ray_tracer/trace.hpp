#ifndef TRACE_H
#define TRACE_H

#include "ray.hpp"
#include "uv.hpp"

Vec3 trace_ray(Ray ray);

struct NormalUv {
    Vec3 normal;
    UV uv;
};

#endif
