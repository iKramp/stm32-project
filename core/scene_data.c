#include "scene_data.h"

static uint8_t *scene_data = 0;
static uint32_t scene_data_size = 0;

void set_scene_data(uint8_t *data, uint32_t size) {
    scene_data = data;
    scene_data_size = size;
}
uint8_t *get_scene_data() {
    return scene_data;
}

uint32_t get_scene_data_size() {
    return scene_data_size;
}
