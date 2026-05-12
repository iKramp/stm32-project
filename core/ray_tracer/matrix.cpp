#include "matrix.hpp"
#include "vec3.hpp"

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
Mat3 Mat3::inverse() {
    Vec3 tmp0 = y_axis.cross(z_axis);
    Vec3 tmp1 = z_axis.cross(x_axis);
    Vec3 tmp2 = x_axis.cross(y_axis);
    float det = z_axis.dot(tmp2);
    float det_recip = 1 / det;
    Vec3 inv_det = Vec3(det_recip, det_recip, det_recip);
    return Mat3::from_cols(tmp0 * inv_det, tmp1 * inv_det, tmp2 * inv_det);
}
Mat3 Mat3::from_cols(Vec3 v0, Vec3 v1, Vec3 v2) {
    Mat3 result;
    result.x_axis = v0;
    result.y_axis = v1;
    result.z_axis = v2;
    return result;
}

Vec3 Affine3::operator*(const Vec3& vec) {
    Vec3 rotated = matrix * vec;
    return rotated;
}
Vec3 Affine3::mul_point(const Vec3& point) {
    Vec3 rotated = matrix * point;
    return rotated + translation;
}
Affine3 Affine3::identity() {
    Affine3 result;
    result.matrix = Mat3::identity();
    result.translation = Vec3(0, 0, 0);
    return result;
}
Affine3& Affine3::translate(const Vec3& offset) {
    this->translation = this->translation + offset;
    return *this;
}

Affine3 Affine3::inverse() {
    Mat3 mat3 = this->matrix.inverse();
    Vec3 translation_ = -(mat3 * this->translation);
    Affine3 ret;
    ret.translation = translation_;
    ret.matrix = mat3;
    return ret;
}
