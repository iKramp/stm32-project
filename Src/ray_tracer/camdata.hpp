#ifndef CAMDATA_H
#define CAMDATA_H

#include <stdint.h>
#include "matrix.hpp"
#include "ray.hpp"

class CamData {
public:
    uint32_t depth;
    struct Affine3 transform;
    uint32_t canvas_width;
    uint32_t canvas_height;
    float fov;
    uint32_t frame;
    uint32_t random_seed;
};

Ray vec_dir_from_cam(CamData &data, float x, float y);

#endif // CAMDATA_H
