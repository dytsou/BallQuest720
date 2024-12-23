#include "../include/Fruit.h"
#include <cstdlib>
#include <ctime>
#include <cmath>

// 假設全域變量已在 main.cpp 中定義
extern float fruitSpeedMultiplier;

Fruit::Fruit(const Vector3& pos) : m_position(pos), m_active(true), m_time(0), m_isRainbow(false), m_points(0) {
    ResetRandomFruit(pos.y, 0.0f); // 初始遊戲時間設為 0.0f
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

    m_position.y -= m_speed * fruitSpeedMultiplier * deltaTime;

    if (m_position.y < -1.0f) {
        m_active = false;
    }
}

void Fruit::ResetRandomFruit(float height, float gameTime) {
    // 設置隨機種子
    static bool seeded = false;
    if (!seeded) {
        srand(static_cast<unsigned>(time(nullptr)));
        seeded = true;
    }

    // 設置水果的位置
    m_position.x = (rand() % 50 - 25) * 1.0f;  // 隨機 x 介於 -25 和 25
    m_position.y = height;
    m_position.z = (rand() % 40 - 20) * 1.0f;  // 隨機 z 介於 -20 和 20

    // 根據遊戲時間決定水果的類型
    if (gameTime <= 60.0f) {
        // 前 80 秒：紅球
        m_color = Vector3(1.0f, 0.0f, 0.0f); // 紅色
        m_size = 0.5f; // 設定適當的大小
        m_points = 1;
        m_isRainbow = false;
    }
    else if (gameTime <= 100.0f) {
        // 81-110 秒：黃球
        m_color = Vector3(1.0f, 1.0f, 0.0f); // 黃色
        m_size = 0.7f; // 設定適當的大小
        m_points = 2;
        m_isRainbow = false;
    }
    else if (gameTime <= 120.0f) {
        // 111-120 秒：彩色球
        // 隨機選擇一種顏色
        float chance = static_cast<float>(rand()) / RAND_MAX;
        if (chance < 0.25f) {
            m_color = Vector3(1.0f, 0.0f, 0.0f); // 紅色
        }
        else if (chance < 0.5f) {
            m_color = Vector3(0.0f, 1.0f, 0.0f); // 綠色
        }
        else if (chance < 0.75f) {
            m_color = Vector3(0.0f, 0.0f, 1.0f); // 藍色
        }
        else {
            m_color = Vector3(1.0f, 0.0f, 1.0f); // 紫色
        }
        m_size = 1.0f; // 設定適當的大小
        m_points = 10;
        m_isRainbow = true; // 啟用彩虹效果
    }

    // 設定速度（根據需要調整）
    m_speed = 5.0f + static_cast<float>(rand() % 30) / 10.0f;  // Speed between 5.0 and 8.0
    m_active = true;
}

void Fruit::DrawSphere() {
    GLUquadricObj* quadric = gluNewQuadric();
    gluQuadricDrawStyle(quadric, GLU_FILL);
    gluSphere(quadric, m_size, 32, 32);
    gluDeleteQuadric(quadric);
}
