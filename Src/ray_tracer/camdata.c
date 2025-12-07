#include "matrix.h"
#include "camdata.h"
#include <math.h>

struct Ray vec_dir_from_cam(struct CamData *data, float x, float y) {
    float fov = (data->fov / 180.0) / 2.0;
    float edge_dist = tanf(fov);
    float pix_x_frac = (x / (float)data->canvas_width) * 2.0 - 1.0;
    float pix_y_frac = (y / (float)data->canvas_height) * 2.0 - 1.0;
    float pix_y_frac_adjusted = pix_y_frac * ((float)data->canvas_height / (float)data->canvas_width);
    float pix_x_dist = pix_x_frac * edge_dist;
    float pix_y_dist = pix_y_frac_adjusted * edge_dist;
    struct Vec3 orientation_vec = {pix_x_dist, pix_y_dist, 1.0f};
    orientation_vec = mul_affine3_vec3(data->transform, orientation_vec);
    struct Vec3 origin = {0, 0, 0};
    struct Ray ret_val = {
        mul_affine3_vec3(data->transform, origin),
        orientation_vec
    };
    return ret_val;
}
