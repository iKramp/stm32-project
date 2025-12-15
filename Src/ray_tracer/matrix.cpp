#include "matrix.hpp"

Mat3 Mat3::operator*(const Mat3& other) {
    Mat3 result;
    result.x_axis = (*this) * other.x_axis;
    result.y_axis = (*this) * other.y_axis;
    result.z_axis = (*this) * other.z_axis;
    return result;
}
Vec3 Mat3::operator*(const Vec3& vec) {
    return Vec3(
        x_axis.x * vec.x + y_axis.x * vec.y + z_axis.x * vec.z,
        x_axis.y * vec.x + y_axis.y * vec.y + z_axis.y * vec.z,
        x_axis.z * vec.x + y_axis.z * vec.y + z_axis.z * vec.z
    );
}
Mat3 Mat3::identity() {
    Mat3 result;
    result.x_axis = Vec3(1, 0, 0);
    result.y_axis = Vec3(0, 1, 0);
    result.z_axis = Vec3(0, 0, 1);
    return result;
}

Vec3 Affine3::operator*(const Vec3& vec) {
    Vec3 rotated = matrix * vec;
    return rotated + translation;
}
Affine3 Affine3::identity() {
    Affine3 result;
    result.matrix = Mat3::identity();
    result.translation = Vec3(0, 0, 0);
    return result;
}
