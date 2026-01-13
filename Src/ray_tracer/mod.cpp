#include "mod.hpp"
#include "bvh.hpp"
#include "instance.hpp"
#include "object.hpp"
#include "ray.hpp"
#include "scene_data.hpp"
#include "trace.hpp"
#include "triangle.hpp"
#include "uv.hpp"
#include "vec3.hpp"
#include "camdata.hpp"
#include "matrix.hpp"
#include "vertex.hpp"

extern "C" {
    #include "../rendering/framebuffer.h"
}

#define SCALING_FACTOR 2

#define QSPI_ADDR 0x90000000

/*
   buffers in order:
   - vertex
   - normal
   - uv
   - triangle
   - bvh
   - object
   - instance
 */

void draw_scaled_pixel(uint32_t x, uint32_t y, uint32_t color) {
    FrameBuffer *fb = get_fb();

    for (int dx = 0; dx < SCALING_FACTOR; dx++) {
        for (int dy = 0; dy < SCALING_FACTOR; dy++) {
            int draw_x = x * SCALING_FACTOR + dx;
            int draw_y = y * SCALING_FACTOR + dy;
            if (draw_x < fb->width && draw_y < fb->height) {
                int index = (draw_y * fb->width + draw_x) * 4;
                *((volatile uint32_t *)(fb->buffer + index)) = color;
            }
        }
    }
}

void tracer_main(volatile uint8_t *framebuffer, uint32_t width, uint32_t height) {
    CamData cam_data = {
        .depth = 5,
        .transform = Affine3::identity().translate(Vec3(0, 0, -10)),
        .canvas_width = width,
        .canvas_height = height,
        .fov = 90.0,
        .frame = 0,
        .random_seed = 42
    };

    uint8_t *data_ptr = (uint8_t *)QSPI_ADDR;

    uint32_t vertex_buffer_size = *((uint32_t *)data_ptr);
    data_ptr += 4;
    Vertex *vertex_buffer = (Vertex *)data_ptr;
    data_ptr += vertex_buffer_size;

    uint32_t normal_buffer_size = *((uint32_t *)data_ptr);
    data_ptr += 4;
    Vec3 *normal_buffer = (Vec3 *)data_ptr;
    data_ptr += normal_buffer_size;

    uint32_t uv_buffer_size = *((uint32_t *)data_ptr);
    data_ptr += 4;
    UV *uv_buffer = (UV *)data_ptr;
    data_ptr += uv_buffer_size;

    uint32_t triangle_buffer_size = *((uint32_t *)data_ptr);
    data_ptr += 4;
    Triangle *triangle_buffer = (Triangle *)data_ptr;
    data_ptr += triangle_buffer_size;

    uint32_t bvh_buffer_size = *((uint32_t *)data_ptr);
    data_ptr += 4;
    BVHNode *bvh_buffer = (BVHNode *)data_ptr;
    data_ptr += bvh_buffer_size;

    uint32_t object_buffer_size = *((uint32_t *)data_ptr);
    data_ptr += 4;
    Object *object_buffer = (Object *)data_ptr;
    data_ptr += object_buffer_size;

    uint32_t instance_buffer_size = *((uint32_t *)data_ptr);
    data_ptr += 4;
    Instance *instance_buffer = (Instance *)data_ptr;
    data_ptr += instance_buffer_size;
    uint32_t instance_count = instance_buffer_size / sizeof(Instance);

    SceneData scene_data;
    scene_data.vert_buffer = vertex_buffer;
    scene_data.normal_buffer = normal_buffer;
    scene_data.uv_buffer = uv_buffer;
    scene_data.triangle_buffer = triangle_buffer;
    scene_data.bvh_buffer = bvh_buffer;
    scene_data.obj_buffer = object_buffer;
    scene_data.instance_buffer = instance_buffer;
    scene_data.instance_count = instance_count;
    SceneData::set_scene_data(scene_data);

    for (int x = width / 4; x < width - width / 4; x += SCALING_FACTOR) {
        for (int y = 0; y < height; y += SCALING_FACTOR) {
            uint32_t color = 0xFF000000;
            Ray cam_ray = vec_dir_from_cam(cam_data, x, y);
            Vec3 ret_col = trace_ray(cam_ray);
            color |= ((uint8_t)(ret_col.x * 255) & 0xFF) << 16;
            color |= ((uint8_t)(ret_col.y * 255) & 0xFF) << 8;
            color |= ((uint8_t)(ret_col.z * 255) & 0xFF);
            draw_scaled_pixel(x / SCALING_FACTOR, y / SCALING_FACTOR, color);
            // for (int dx = 0; dx < SCALING_FACTOR; dx++) {
            //     for (int dy = 0; dy < SCALING_FACTOR; dy++) {
            //         int draw_x = x + dx;
            //         int draw_y = y + dy;
            //         if (draw_x < width && draw_y < height) {
            //             int index = (draw_y * width + draw_x) * 4;
            //             *((volatile uint32_t *)(framebuffer + index)) = color;
            //         }
            //     }
            // }
        }
    }
}
