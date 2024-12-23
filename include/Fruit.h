#ifndef FRUIT_H
#define FRUIT_H

#include <GL/glu.h>  // Include GLU here for GLUquadricObj
#include "Vector3.h"

enum class FruitType {
    MAIN,
    BLACK
};

class Fruit {
public:
    Fruit(const Vector3& pos, FruitType type);
    ~Fruit(); // Destructor (optional)
    void Draw();
    void Update(float deltaTime);
    void ResetRandomFruit(float height, float gameTime, FruitType type);

    // Getter and Setter
    bool IsActive() const { return m_active; }
    void SetActive(bool active) { m_active = active; }
    Vector3 GetPosition() const { return m_position; }
    int GetPoints() const { return m_points; }

    // Public static cleanup function
    static void CleanupQuadric();

private:
    void DrawSphere();

    Vector3 m_position;
    Vector3 m_color;
    float m_size;
    float m_speed;
    bool m_active;
    float m_time;
    bool m_isRainbow;
    int m_points;
    FruitType m_type; // Added fruit type

    static GLUquadricObj* s_quadric;  // Static quadric object
};

#endif // FRUIT_H
