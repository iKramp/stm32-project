#include "ray.hpp"

void Ray::normalize() {
    direction = direction.normalize();
}
