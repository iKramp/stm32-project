#ifndef VEC3_H
#define VEC3_H

class Vec3 {
public:
    float x;
    float y;
    float z;
    
    Vec3();
    Vec3(float x, float y, float z);
    Vec3 operator+(const Vec3& other);
    Vec3 operator-(const Vec3& other);
    Vec3 operator*(float scalar);
    Vec3 operator*(Vec3 rhs);
    Vec3 operator/(Vec3 rhs);
    Vec3 operator-();

    float dot(const Vec3& other);
    Vec3 cross(const Vec3& other);

    float length();
    float length2();
    Vec3 normalize();
};

#endif // VEC3_H
