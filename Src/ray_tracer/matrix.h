#ifndef MATRIX_H
#define MATRIX_H

#include <math.h>
#include "vec3.h"

//follow rust's GLAM lib convention
struct Mat3 {
    struct Vec3 x_axis;
    struct Vec3 y_axis;
    struct Vec3 z_axis;
};

struct Affine3 {
    struct Mat3 matrix;
    struct Vec3 translation;
};

struct Vec3 mul_mat3_vec3(struct Mat3 matrix, struct Vec3 vec);
struct Vec3 mul_affine3_vec3(struct Affine3 matrix, struct Vec3 vec);

#endif // MATRIX_H
