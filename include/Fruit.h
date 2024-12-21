#ifndef FRUIT_H
#define FRUIT_H

#include "Vector3.h"
#include <GL/glut.h>

class Fruit {
public:
    Fruit(const Vector3& pos);
    void Draw();
    void Update(float deltaTime);
    void ResetRandomFruit(float height);
    bool IsActive() const { return m_active; }
    void SetActive(bool active) { m_active = active; }
    Vector3 GetPosition() const { return m_position; }
    int GetPoints() const { return m_points; }

private:
    Vector3 m_position;
    Vector3 m_color;
    float m_size;
    float m_speed;
    int m_points;
    bool m_active;
    bool m_isRainbow;
    float m_time;

    void DrawSphere();
};

#endif // FRUIT_H 