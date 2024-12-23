#ifndef FRUIT_H
#define FRUIT_H

#include "Vector3.h" // 假設有一個 Vector3 類別
#include <GL/glut.h>

class Fruit {
public:
    Fruit(const Vector3& pos);
    void Draw();
    void Update(float deltaTime);
    bool IsActive() const { return m_active; }
    Vector3 GetPosition() const { return m_position; }
    int GetPoints() const { return m_points; }
    void SetActive(bool active) { m_active = active; }
    void ResetRandomFruit(float height, float gameTime); // 修改後的聲明

private:
    Vector3 m_position;
    Vector3 m_color;
    float m_size;
    float m_speed;
    bool m_active;
    bool m_isRainbow;
    int m_points;
    float m_time;

    void DrawSphere();
};

#endif // FRUIT_H
