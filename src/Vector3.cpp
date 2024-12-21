#include "../include/Vector3.h"
#include <cmath>

void Vector3::Normalize() {
    float length = std::sqrt(x * x + y * y + z * z);
    if (length != 0) {
        x /= length;
        y /= length;
        z /= length;
    }
} 