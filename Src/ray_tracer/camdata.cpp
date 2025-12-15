#include "matrix.hpp"
#include "camdata.hpp"
#include "math_wrapper.hpp"


Ray vec_dir_from_cam(CamData &data, float x, float y) {
    float fov = (data.fov / 180.0) / 2.0;
    float edge_dist = tanf(fov);
    float pix_x_frac = (x / (float)data.canvas_width) * 2.0 - 1.0;
    float pix_y_frac = (y / (float)data.canvas_height) * 2.0 - 1.0;
    float pix_y_frac_adjusted = pix_y_frac * ((float)data.canvas_height / (float)data.canvas_width);
    float pix_x_dist = pix_x_frac * edge_dist;
    float pix_y_dist = pix_y_frac_adjusted * edge_dist;
    Vec3 orientation_vec = {pix_x_dist, pix_y_dist, 1.0f};
    orientation_vec = data.transform * orientation_vec;
    Vec3 origin = {0, 0, 0};
    Ray ret_val = {
        data.transform * origin,
        orientation_vec
    };
    return ret_val;
}
