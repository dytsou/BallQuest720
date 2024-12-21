#include "../include/Fruit.h"
#include <cstdlib>
#include <ctime>
#include <cmath>

Fruit::Fruit(const Vector3& pos) : m_position(pos), m_active(true), m_time(0) {
    ResetRandomFruit(pos.y);
}

void Fruit::Draw() {
    if (!m_active) return;

    glPushMatrix();
    glTranslatef(m_position.x, m_position.y, m_position.z);

    if (m_isRainbow) {
        // Rainbow effect
        m_time += 0.01f;
        float r = sin(m_time * 2.0f) * 0.5f + 0.5f;
        float g = sin(m_time * 2.0f + 2.094f) * 0.5f + 0.5f;
        float b = sin(m_time * 2.0f + 4.189f) * 0.5f + 0.5f;
        glColor3f(r, g, b);
    }
    else {
        glColor3f(m_color.x, m_color.y, m_color.z);
    }

    DrawSphere();
    glPopMatrix();
}

void Fruit::Update(float deltaTime) {
    if (!m_active) return;

    m_position.y -= m_speed * deltaTime;

    if (m_position.y < -1.0f) {
        m_active = false;
    }
}

void Fruit::ResetRandomFruit(float height) {
    static bool seeded = false;
    if (!seeded) {
        srand(static_cast<unsigned>(time(nullptr)));
        seeded = true;
    }

    m_position.x = (rand() % 50 - 25) * 1.0f;  // Random x between -25 and 25
    m_position.y = height;
    m_position.z = (rand() % 40 - 20) * 1.0f;  // Random z between -20 and 20

    float chance = static_cast<float>(rand()) / RAND_MAX;
    if (chance < 0.1f) { // Rainbow ball (10%)
        m_color = Vector3(1.0f, 1.0f, 1.0f);
        m_size = 0.3f;
        m_points = 10;
        m_isRainbow = true;
    }
    else if (chance < 0.2f) { // Black ball (10%)
        m_color = Vector3(0.0f, 0.0f, 0.0f);
        m_size = 2.0f;
        m_points = -10;
        m_isRainbow = false;
    }
    else if (chance < 0.4f) { // Blue ball (20%)
        m_color = Vector3(0.0f, 0.0f, 1.0f);
        m_size = 0.5f;
        m_points = 5;
        m_isRainbow = false;
    }
    else if (chance < 0.7f) { // Green ball (30%)
        m_color = Vector3(0.0f, 1.0f, 0.0f);
        m_size = 1.0f;
        m_points = 3;
        m_isRainbow = false;
    }
    else { // Red ball (30%)
        m_color = Vector3(1.0f, 0.0f, 0.0f);
        m_size = 1.5f;
        m_points = 1;
        m_isRainbow = false;
    }

    m_speed = 5.0f + static_cast<float>(rand() % 30) / 10.0f;  // Speed between 5 and 8
    m_active = true;
}

void Fruit::DrawSphere() {
    GLUquadricObj* quadric = gluNewQuadric();
    gluQuadricDrawStyle(quadric, GLU_FILL);
    gluSphere(quadric, m_size, 32, 32);
    gluDeleteQuadric(quadric);
} 