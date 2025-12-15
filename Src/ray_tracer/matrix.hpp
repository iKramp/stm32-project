#ifndef MATRIX_H
#define MATRIX_H

#include "vec3.hpp"

//follow rust's GLAM lib convention
class Mat3 {
    struct Vec3 x_axis;
    struct Vec3 y_axis;
    struct Vec3 z_axis;

public:
    Mat3 operator*(const Mat3& other);
    Vec3 operator*(const Vec3& vec);
    static Mat3 identity();
};

class Affine3 {
    struct Mat3 matrix;
    struct Vec3 translation;

public:
    Vec3 operator*(const Vec3& vec);
    static Affine3 identity();
};

#endif // MATRIX_H
