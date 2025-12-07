#include "ray.h"

void ray_normalize(struct Ray *ray) {
    ray->direction = vec3_normalize(ray->direction);
}
