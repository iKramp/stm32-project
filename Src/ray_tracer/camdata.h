#ifndef CAMDATA_H
#define CAMDATA_H

#include <stdint.h>
#include "matrix.h"
#include "vec3.h"
#include "ray.h"

struct CamData {
    uint32_t depth;
    struct Affine3 transform;
    uint32_t canvas_width;
    uint32_t canvas_height;
    float fov;
    uint32_t frame;
    uint32_t random_seed;
};

struct Ray vec_dir_from_cam(struct CamData *data, float x, float y);

#endif // CAMDATA_H
