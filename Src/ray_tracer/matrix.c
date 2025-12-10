#include "matrix.h"

struct Mat3 mat3_identity() {
    struct Mat3 mat;
    mat.x_axis = (struct Vec3){1.0, 0.0, 0.0};
    mat.y_axis = (struct Vec3){0.0, 1.0, 0.0};
    mat.z_axis = (struct Vec3){0.0, 0.0, 1.0};
    return mat;
}

struct Affine3 affine3_identity() {
    struct Affine3 aff;
    aff.matrix = mat3_identity();
    aff.translation = (struct Vec3){0.0, 0.0, 0.0};
    return aff;
}

struct Vec3 mul_mat3_vec3(struct Mat3 matrix, struct Vec3 vec) {
    struct Vec3 result;
    result.x = matrix.x_axis.x * vec.x + matrix.y_axis.x * vec.y + matrix.z_axis.x * vec.z;
    result.y = matrix.x_axis.y * vec.x + matrix.y_axis.y * vec.y + matrix.z_axis.y * vec.z;
    result.z = matrix.x_axis.z * vec.x + matrix.y_axis.z * vec.y + matrix.z_axis.z * vec.z;
    return result;
}

struct Vec3 mul_affine3_vec3(struct Affine3 matrix, struct Vec3 vec) {
    struct Vec3 rotated = mul_mat3_vec3(matrix.matrix, vec);
    struct Vec3 result;
    result.x = rotated.x + matrix.translation.x;
    result.y = rotated.y + matrix.translation.y;
    result.z = rotated.z + matrix.translation.z;
    return result;
}
