#ifndef MATRIX_H
#define MATRIX_H

#include "vec3.hpp"
#include <stdint.h>

//follow rust's GLAM lib convention
class Mat3 {
    struct Vec3 x_axis;
    uint32_t padding_1;
    struct Vec3 y_axis;
    uint32_t padding_2;
    struct Vec3 z_axis;
    uint32_t padding_3;

public:
    Mat3 operator*(const Mat3& other);
    Vec3 operator*(const Vec3& vec);
    Mat3 from_cols(Vec3 v0, Vec3 v1, Vec3 v2);
    static Mat3 identity();
    Mat3 inverse();
};

class Affine3 {
    struct Mat3 matrix;
    struct Vec3 translation;
    uint32_t padding;

public:
    Vec3 operator*(const Vec3& vec);
    Vec3 mul_point(const Vec3& point);
    Affine3& translate(const Vec3& offset);
    static Affine3 identity();
    Affine3 inverse();
};

#endif // MATRIX_H
