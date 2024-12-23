#include "Fruit.h"
#include <cstdlib>
#include <ctime>
#include <cmath>

// Assume global variable defined in main.cpp
extern float fruitSpeedMultiplier;

// Initialize the static member
GLUquadricObj* Fruit::s_quadric = nullptr;

Fruit::Fruit(const Vector3& pos, FruitType type) 
    : m_position(pos), m_active(true), m_time(0), m_isRainbow(false), m_points(0), m_type(type) {
    ResetRandomFruit(pos.y, 0.0f, type); // Initial game time set to 0.0f

    // Initialize the quadric if not already done
    if (!s_quadric) {
        s_quadric = gluNewQuadric();
        gluQuadricDrawStyle(s_quadric, GLU_FILL);
    }
}

Fruit::~Fruit() {
    // Destructor can remain empty if CleanupQuadric is called manually
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

void Fruit::DrawSphere() {
    if (!s_quadric) return; // Ensure quadric is initialized
    gluSphere(s_quadric, m_size, 32, 32);
}

void Fruit::Update(float deltaTime) {
    if (!m_active) return;

    m_position.y -= m_speed * fruitSpeedMultiplier * deltaTime;

    if (m_position.y < -1.0f) {
        m_active = false;
    }
}

void Fruit::ResetRandomFruit(float height, float gameTime, FruitType type) {
    // Set random seed
    static bool seeded = false;
    if (!seeded) {
        srand(static_cast<unsigned>(time(nullptr)));
        seeded = true;
    }

    m_type = type;

    // Set fruit position
    m_position.x = (rand() % 50 - 25) * 1.0f;  // Random x between -25 and 25
    m_position.y = height;
    m_position.z = (rand() % 40 - 20) * 1.0f;  // Random z between -20 and 20

    if (m_type == FruitType::BLACK) {
        // Set black fruit attributes
        m_color = Vector3(0.0f, 0.0f, 0.0f); // Black
        m_size = 0.5f; // Black fruit size
        m_points = -1; // Assuming black fruit deducts points
        m_isRainbow = false;
    }
    else { // FruitType::MAIN
        // Determine main fruit type based on game time
        if (gameTime <= 60.0f) {
            // First 60 seconds: Red fruit
            m_color = Vector3(1.0f, 0.0f, 0.0f); // Red
            m_size = 0.5f; // Appropriate size
            m_points = 1;
            m_isRainbow = false;
        }
        else if (gameTime <= 100.0f) {
            // 60-100 seconds: Yellow fruit
            m_color = Vector3(1.0f, 1.0f, 0.0f); // Yellow
            m_size = 0.7f; // Appropriate size
            m_points = 2;
            m_isRainbow = false;
        }
        else if (gameTime <= 120.0f) {
            // 100-120 seconds: Rainbow fruit
            // Randomly choose a color
            float chance = static_cast<float>(rand()) / RAND_MAX;
            if (chance < 0.25f) {
                m_color = Vector3(1.0f, 0.0f, 0.0f); // Red
            }
            else if (chance < 0.5f) {
                m_color = Vector3(0.0f, 1.0f, 0.0f); // Green
            }
            else if (chance < 0.75f) {
                m_color = Vector3(0.0f, 0.0f, 1.0f); // Blue
            }
            else {
                m_color = Vector3(1.0f, 0.0f, 1.0f); // Purple
            }
            m_size = 1.0f; // Appropriate size
            m_points = 10;
            m_isRainbow = true; // Enable rainbow effect
        }
    }

    // Set speed (adjust as needed)
    m_speed = 5.0f + static_cast<float>(rand() % 30) / 10.0f;  // Speed between 5.0 and 8.0
    m_active = true;
}

void Fruit::CleanupQuadric() {
    if (s_quadric) {
        gluDeleteQuadric(s_quadric);
        s_quadric = nullptr;
    }
}
