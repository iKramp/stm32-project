#include "mod.h"
#include "ray.h"
#include "trace.h"
#include "vec3.h"
#include "camdata.h"

void tracer_main(volatile uint8_t *framebuffer, int width, int height) {
    struct CamData cam_data = {
        .depth = 5,
        .transform = affine3_identity(),
        .canvas_width = width,
        .canvas_height = height,
        .fov = 90.0,
        .frame = 0,
        .random_seed = 42
    };

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            int index = (y * width + x) * 4;
            uint32_t color = 0xFF000000;
            struct Ray cam_ray = vec_dir_from_cam(&cam_data, x, y);
            struct Vec3 ret_col = trace_ray(cam_ray);
            color |= ((uint8_t)(ret_col.x * 255) & 0xFF) << 16;
            color |= ((uint8_t)(ret_col.y * 255) & 0xFF) << 8;
            color |= ((uint8_t)(ret_col.z * 255) & 0xFF);
            *((volatile uint32_t *)(framebuffer + index)) = color;
        }
    }
}
