#include "hit_record.hpp"
#include "math_wrapper.hpp"
#include <stdint.h>
#include <stdio.h>

extern "C" {
    #include "../rendering/fb_text.h"
}

void HitRecord::try_add_hit(float t_hit, int object_index, int sub_index_) {
    if (t_hit < t) {
        t = t_hit;
        obj_index = object_index;
        this->sub_index = sub_index_;
    }
}
