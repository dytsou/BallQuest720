#ifndef VECTOR3_H
#define VECTOR3_H

class Vector3 {
public:
    float x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(float X, float Y, float Z) : x(X), y(Y), z(Z) {}

    Vector3 operator+(const Vector3& v) const {
        return Vector3(x + v.x, y + v.y, z + v.z);
    }

    Vector3 operator-(const Vector3& v) const {
        return Vector3(x - v.x, y - v.y, z - v.z);
    }

    Vector3 operator*(float scale) const {
        return Vector3(x * scale, y * scale, z * scale);
    }

    Vector3 Cross(const Vector3& v) const {
        return Vector3(y * v.z - z * v.y,
                      z * v.x - x * v.z,
                      x * v.y - y * v.x);
    }

    float Dot(const Vector3& v) const {
        return x * v.x + y * v.y + z * v.z;
    }

    void Normalize();
};

#endif // VECTOR3_H 