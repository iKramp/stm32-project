#pragma once


class HitRecord {
public:
    float t;
    int obj_index;
    int sub_index;
    HitRecord() : t(100000000), obj_index(-1), sub_index(-1) {}
    void try_add_hit(float t_hit, int object_index, int sub_index_);
};
