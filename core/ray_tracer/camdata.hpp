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
    uint32_t top_left_x;
    uint32_t top_left_y;
    uint32_t region_width;
    uint32_t region_height;
    float fov;
    uint32_t frame;
    uint32_t random_seed;
};

Ray vec_dir_from_cam(CamData &data, float x, float y);
CamData get_cam_data();

#endif // CAMDATA_H
