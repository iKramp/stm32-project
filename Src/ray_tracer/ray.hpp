#ifndef RAY_H
#define RAY_H

#include "vec3.hpp"

class Ray {
public:
    Vec3 origin;
    Vec3 direction;
    Ray(const Vec3& origin, const Vec3& direction) : origin(origin), direction(direction) {}
    void normalize();
};

void ray_normalize(Ray &ray);

#endif // RAY_H
