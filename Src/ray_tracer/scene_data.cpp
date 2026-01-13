#include "scene_data.hpp"

static SceneData scene_data_instance;

SceneData& SceneData::get_scene_data() {
    return scene_data_instance;
}

void SceneData::set_scene_data(SceneData& scene_data) {
    scene_data_instance = scene_data;
}
