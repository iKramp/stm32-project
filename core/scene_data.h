#ifndef COMMON_SCENE_DATA_H
#define COMMON_SCENE_DATA_H

#include <stdint.h>

void set_scene_data(uint8_t *data, uint32_t size);
uint8_t *get_scene_data();
uint32_t get_scene_data_size();

#endif // COMMON_SCENE_DATA_H
