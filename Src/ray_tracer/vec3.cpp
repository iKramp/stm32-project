#include "vec3.hpp"
#include "math_wrapper.hpp"


Vec3::Vec3() : x(0), y(0), z(0) {}
Vec3::Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

Vec3 Vec3::operator+(const Vec3& other) {
    return Vec3(x + other.x, y + other.y, z + other.z);
}
Vec3 Vec3::operator-(const Vec3& other) {
    return Vec3(x - other.x, y - other.y, z - other.z);
}
Vec3 Vec3::operator*(float scalar) {
    return Vec3(x * scalar, y * scalar, z * scalar);
}

float Vec3::dot(const Vec3& other) {
    return x * other.x + y * other.y + z * other.z;
}
Vec3 Vec3::cross(const Vec3& other) {
    return Vec3(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}

float Vec3::length() {
    return sqrtf(this->length2());
}
float Vec3::length2() {
    return x * x + y * y + z * z;
}
Vec3 Vec3::normalize() {
    float len = length();
    if (len == 0) {
        return Vec3(0, 0, 0);
    }
    return Vec3(x / len, y / len, z / len);
}
