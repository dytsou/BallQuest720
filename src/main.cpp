#include <cmath>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iomanip>    // 為了使用 std::fixed 和 std::setprecision
#include <GL/glut.h>
#include "../include/Camera.h"
#include "../include/Fruit.h"
#include "../include/Texture.h"
#include "../include/Text.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace std;

// ========================== 全域參數宣告 ========================== //

// 視窗大小
const int WINDOW_WIDTH  = 1280;
const int WINDOW_HEIGHT = 720;

// 地面與牆面
const float GROUND_SIZE = 50.0f;
const float GROUND_Y    = 0.0f;
const float WALL_HEIGHT = 10.0f;
const float WALL_DISTANCE = GROUND_SIZE;  // 牆的位置（與地面邊緣相同）
CTexture wallTexture;                     // 牆面紋理

// 攝影機
CCamera camera;

// HUD & 分數
Text    scoreText;
int     score = 0;
int     life  = 20;  // 初始生命
vector<Fruit> fruits;

// 水果相關
const int  BallHeight = 50;  // 水果重置時離地高度
const int  Interval   = 20;  // 未使用，可自行擴充

// 籃子 (使用 code2 較複雜的編織籃子)
const float BASKET_RADIUS   = 0.1f;
const float BASKET_HEIGHT   = 0.1f;
const int   BASKET_SEGMENTS = 32;
Vector3 basketPosition(0.0f, 1.0f, 0.0f); // 籃子位置

// 滑鼠、鍵盤輸入
bool keyStates[256] = {false};
bool specialKeyStates[256] = {false};
int  lastMouseX = WINDOW_WIDTH / 2;
int  lastMouseY = WINDOW_HEIGHT / 2;
bool firstMouse = true;
float yaw   = -90.0f;
float pitch = 0.0f;

// 遊戲狀態 & 難度選擇
enum GameState {
    MENU,
    PLAYING,
    GAMEOVER
};
enum Difficulty {
    EASY,
    NORMAL,
    HARD
};

// 與選單相關的按鈕結構
struct Button {
    float x, y, width, height;
    string text;
    bool isHovered;
};

// 選單按鈕
vector<Button> difficultyButtons = {
    {WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 - 80, 200, 50, "Easy",   false},
    {WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2,      200, 50, "Normal", false},
    {WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 + 80, 200, 50, "Hard",   false}
};

GameState   currentState        = MENU;
Difficulty  selectedDifficulty  = NORMAL;

// ============ 以下為 code2 的計時機制與參數調整功能 ============ //

// 遊戲時間
float gameTime          = 0.0f;         // 已經過的時間（秒）
const float GAME_DURATION = 120.0f;     // 總時長（秒）
float gameOverStartTime = 0.0f;         // 記錄遊戲結束的時間點

// 速度、靈敏度等可調參數
float cameraSpeed         = 0.5f; // 攝影機移動速度
float mouseSensitivity    = 0.05f; // 滑鼠視角旋轉靈敏度（合併時取 code1 的 0.05f 與 code2 的 0.01f 中間值）
float fruitSpeedMultiplier = 1.0f; // 水果下落速度倍增器

// ========================== 函式宣告 ========================== //
void init();
void display();
void reshape(int w, int h);
void update();
void keyboard(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void specialKeys(int key, int x, int y);
void mouse(int button, int state, int x, int y);
void mouseMotion(int x, int y);
void passiveMotion(int x, int y);

void createGroundAndWalls();
void drawMenu();
void drawButton(const Button& btn);
void startGame(Difficulty diff);

void processKeys();
void checkCollisions();
//void drawBasket();

// ========================== 主函式 ========================== //

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("BallQuest 720 - Combined Version");

    init();

    // 設定 callback
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);
    glutPassiveMotionFunc(passiveMotion);
    glutIdleFunc(update);

    // 選單狀態下顯示滑鼠指標
    glutSetCursor(GLUT_CURSOR_LEFT_ARROW);

    glutMainLoop();
    return 0;
}

// ========================== 初始化相關 ========================== //

void init() {
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    // 載入牆面紋理
    wallTexture.LoadTexture("../textures/wall.bmp");

    // 啟用照明
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // 設定光源
    GLfloat lightPosition[] = { 0.0f, 10.0f, 0.0f, 1.0f };
    GLfloat lightAmbient[]  = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat lightDiffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiffuse);

    // 初始化攝影機
    camera.PositionCamera(
        0.0f, 2.0f, 6.0f,   // 位置
        0.0f, 0.0f, 0.0f,   // View
        0.0f, 1.0f, 0.0f    // Up
    );

    // 初始化亂數
    srand(static_cast<unsigned>(time(nullptr)));

    // 預設遊戲時間歸零
    gameTime          = 0.0f;
    gameOverStartTime = 0.0f;
}

// ========================== 視窗與顯示相關 ========================== //

void reshape(int w, int h) {
    if (h == 0) h = 1;
    float ratio = (float)w / (float)h;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, ratio, 0.1f, 1000.0f);
    glMatrixMode(GL_MODELVIEW);
}

void display() {
    if (currentState == MENU) {
        // 選單畫面
        drawMenu();
        return;
    }
    else if (currentState == GAMEOVER) {
        // GAMEOVER 畫面
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        // 顯示「Game Over」與最終分數
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 0.0f, 0.0f);
        string gameOverText    = "Game Over";
        string finalScoreText  = "Final Score: " + to_string(score);

        // 在螢幕中央顯示
        scoreText.RenderText(WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2,     gameOverText);
        glColor3f(1.0f, 1.0f, 1.0f);
        scoreText.RenderText(WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2 - 40, finalScoreText);

        glutSwapBuffers();
        return;
    }

    // 否則就是 PLAYING 中
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  // code1 原本設定白色背景
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    camera.Look();

    // 繪製地面與四面牆
    createGroundAndWalls();

    // 繪製籃子
    //drawBasket();

    // 繪製水果
    for (auto& fruit : fruits) {
        fruit.Draw();
    }

    // 繪製 HUD（分數、生命、剩餘時間）
    glDisable(GL_LIGHTING);
    glColor3f(0.0f, 0.0f, 0.0f);

    // 分數與生命
    stringstream ss;
    ss << "Score: " << score << "  Life: " << life;
    scoreText.RenderText(10, 30, ss.str());

    // 剩餘時間
    float remainingTime = GAME_DURATION - gameTime;
    if (remainingTime < 0.0f) remainingTime = 0.0f;
    stringstream timeSS;
    timeSS << fixed << setprecision(1) << "Time: " << remainingTime << " sec";
    scoreText.RenderText(10, 60, timeSS.str());

    glEnable(GL_LIGHTING);

    glutSwapBuffers();
}

// ========================== 遊戲流程、更新邏輯 ========================== //

void update() {
    static float lastTime = 0.0f;
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; // 轉秒
    float deltaTime   = currentTime - lastTime;
    lastTime          = currentTime;

    if (currentState == MENU) {
        // 若在選單中，只要不斷重繪
        glutPostRedisplay();
        return;
    }
    else if (currentState == GAMEOVER) {
        // 等待顯示一段時間或自行退出
        // 若需要自動結束，可在此判斷
        glutPostRedisplay();
        return;
    }

    // ============= 以下為 PLAYING 狀態下的更新邏輯 ============= //

    // 更新遊戲計時
    gameTime += deltaTime;
    if (gameTime >= GAME_DURATION) {
        // 時間到 -> 切換到 GAMEOVER
        currentState       = GAMEOVER;
        gameOverStartTime  = currentTime;
        return;
    }

    // 處理鍵盤輸入 (WASD 等)
    processKeys();

    // 更新籃子位置 (跟隨玩家視線方向前方 0.7f)
    Vector3 forward = camera.m_vView - camera.m_vPosition;
    forward.y = 0;
    forward.Normalize();
    forward = forward * 0.7f;

    basketPosition.x = camera.m_vPosition.x + forward.x;
    basketPosition.z = camera.m_vPosition.z + forward.z;
    basketPosition.y = camera.m_vPosition.y + 0.1f;

    // 更新水果
    for (auto& fruit : fruits) {
        fruit.Update(deltaTime * fruitSpeedMultiplier); 
    }

    // 檢查碰撞
    checkCollisions();

    // 重置不活躍水果
    for (auto& fruit : fruits) {
        if (!fruit.IsActive()) {
            fruit.ResetRandomFruit(BallHeight, gameTime); 
        }
    }

    glutPostRedisplay();
}

// ========================== 選單相關 ========================== //

void drawMenu() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // 設定深色背景
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 標題與說明文字
    const int charWidth = 15;
    string titleText    = "Select Difficulty";
    string instructText1= "Use WASD to move, Mouse to look around";
    string instructText2= "Press Q to quit game";

    int titleWidth   = titleText.size()    * charWidth;
    int instruct1Width = instructText1.size() * charWidth;
    int instruct2Width = instructText2.size() * charWidth;

    glDisable(GL_LIGHTING);
    glColor3f(0.9f, 0.9f, 0.9f);  // 標題顏色
    scoreText.RenderText(WINDOW_WIDTH/2 - titleWidth/2 + 55, WINDOW_HEIGHT/3 - 10, titleText);

    // 繪製按鈕
    for (const auto& btn : difficultyButtons) {
        drawButton(btn);
    }

    // 繪製說明文字（黃色）
    glColor3f(1.0f, 1.0f, 0.0f);
    scoreText.RenderText(WINDOW_WIDTH/2 - instruct1Width/2 + 90, WINDOW_HEIGHT - 100, instructText1);
    scoreText.RenderText(WINDOW_WIDTH/2 - instruct2Width/2 + 60, WINDOW_HEIGHT - 70,  instructText2);

    glutSwapBuffers();
}

void drawButton(const Button& btn) {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    if (btn.isHovered) {
        glColor4f(0.4f, 0.7f, 1.0f, 1.0f);  // 高亮藍
    } else {
        glColor4f(0.1f, 0.3f, 0.6f, 1.0f);  // 深藍
    }
    // 按鈕背景
    glBegin(GL_QUADS);
        glVertex2f(btn.x,            btn.y);
        glVertex2f(btn.x+btn.width,  btn.y);
        glVertex2f(btn.x+btn.width,  btn.y+btn.height);
        glVertex2f(btn.x,            btn.y+btn.height);
    glEnd();

    // 按鈕邊框（白色）
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(btn.x,            btn.y);
        glVertex2f(btn.x+btn.width,  btn.y);
        glVertex2f(btn.x+btn.width,  btn.y+btn.height);
        glVertex2f(btn.x,            btn.y+btn.height);
    glEnd();

    // 按鈕文字
    glColor3f(1.0f, 1.0f, 1.0f);
    float textX = btn.x + (btn.width - btn.text.length() * 9) / 2.0f; 
    float textY = btn.y + (btn.height + 10) / 2.0f;
    scoreText.RenderText(textX, textY, btn.text);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

// 開始遊戲：設定難度、初始化分數、時間等
void startGame(Difficulty diff) {
    currentState       = PLAYING;
    selectedDifficulty = diff;
    score             = 0;
    gameTime          = 0.0f;
    gameOverStartTime = 0.0f;

    // 依難度設定
    fruits.clear();
    switch (diff) {
        case EASY:
            life = 30;
            for (int i = 0; i < 3; ++i) {
                fruits.push_back(Fruit(Vector3(0, BallHeight + 15*i, 0)));
            }
            break;
        case NORMAL:
            life = 20;
            for (int i = 0; i < 5; ++i) {
                fruits.push_back(Fruit(Vector3(0, BallHeight + 15*i, 0)));
            }
            break;
        case HARD:
            life = 10;
            for (int i = 0; i < 7; ++i) {
                fruits.push_back(Fruit(Vector3(0, BallHeight + 15*i, 0)));
            }
            break;
    }

    // 重設攝影機
    camera.PositionCamera(0.0f, 2.0f, 6.0f,
                         0.0f, 0.0f, 0.0f,
                         0.0f, 1.0f, 0.0f);

    // 隱藏滑鼠並置中
    glutSetCursor(GLUT_CURSOR_NONE);
    glutWarpPointer(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
    firstMouse = true;
}

// ========================== 事件回呼 ========================== //

void mouse(int button, int state, int x, int y) {
    // 只有在選單狀態接受滑鼠點擊
    if (currentState == MENU && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        for (size_t i = 0; i < difficultyButtons.size(); ++i) {
            const auto& btn = difficultyButtons[i];
            if (x >= btn.x && x <= btn.x + btn.width &&
                y >= btn.y && y <= btn.y + btn.height) {
                startGame(static_cast<Difficulty>(i));
                break;
            }
        }
    }
}

void passiveMotion(int x, int y) {
    if (currentState == MENU) {
        // 更新按鈕 hover 狀態
        for (auto& btn : difficultyButtons) {
            btn.isHovered = 
                (x >= btn.x && x <= btn.x + btn.width &&
                 y >= btn.y && y <= btn.y + btn.height);
        }
        glutPostRedisplay();
    }
    else if (currentState == PLAYING) {
        // 若在遊戲中，轉為第一人稱視角處理
        mouseMotion(x, y);
    }
}

// 第一人稱視角滑鼠移動
void mouseMotion(int x, int y) {
    if (currentState != PLAYING) return;

    if (firstMouse) {
        lastMouseX = x;
        lastMouseY = y;
        firstMouse = false;
    }

    float xoffset = x - lastMouseX;
    float yoffset = lastMouseY - y;
    lastMouseX = x;
    lastMouseY = y;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    // 限制 pitch
    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    // 計算新方向
    Vector3 direction;
    direction.x = cos(yaw * 0.0174532925f) * cos(pitch * 0.0174532925f);
    direction.y = sin(pitch * 0.0174532925f);
    direction.z = sin(yaw * 0.0174532925f) * cos(pitch * 0.0174532925f);
    direction.Normalize();

    camera.m_vView = camera.m_vPosition + direction;

    // 保持游標置中
    if (x != WINDOW_WIDTH/2 || y != WINDOW_HEIGHT/2) {
        glutWarpPointer(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
        lastMouseX = WINDOW_WIDTH/2;
        lastMouseY = WINDOW_HEIGHT/2;
    }
}

void specialKeys(int key, int x, int y) {
    // 預留特殊按鍵
}

// 鍵盤按下
void keyboard(unsigned char key, int x, int y) {
    keyStates[key] = true;

    // 不管在哪個 state，按 q/Q 就退出
    if (key == 'q' || key == 'Q') {
        exit(0);
    }
    // 測試用：按 z/Z 顯示分數並退出
    else if (key == 'z' || key == 'Z') {
        cout << "Final Score: " << score << endl;
        exit(0);
    }

    if (currentState == PLAYING) {
        // 只有在遊戲進行中才處理這些
        if (key == '+' || key == '=') {
            cameraSpeed         += 0.1f;
            fruitSpeedMultiplier+= 0.1f;
            cout << "Camera Speed: " << cameraSpeed
                 << ", Fruit Speed Multiplier: " << fruitSpeedMultiplier << endl;
        }
        else if (key == '-' || key == '_') {
            cameraSpeed          = max(0.1f, cameraSpeed - 0.1f);
            fruitSpeedMultiplier = max(0.1f, fruitSpeedMultiplier - 0.1f);
            cout << "Camera Speed: " << cameraSpeed
                 << ", Fruit Speed Multiplier: " << fruitSpeedMultiplier << endl;
        }
        else if (key == '[') {
            mouseSensitivity += 0.01f;
            cout << "Mouse Sensitivity: " << mouseSensitivity << endl;
        }
        else if (key == ']') {
            mouseSensitivity = max(0.01f, mouseSensitivity - 0.01f);
            cout << "Mouse Sensitivity: " << mouseSensitivity << endl;
        }
    }
}

// 鍵盤彈起
void keyboardUp(unsigned char key, int x, int y) {
    keyStates[key] = false;
}

// ========================== 核心處理函式 ========================== //

// 鍵盤處理：WASD 移動
void processKeys() {
    if (currentState != PLAYING) return;

    float speed = cameraSpeed;
    // 空白鍵衝刺
    if (keyStates[' ']) {
        speed *= 2.0f;
    }

    Vector3 forward = camera.m_vView - camera.m_vPosition;
    forward.y = 0;
    forward.Normalize();

    Vector3 right = forward.Cross(camera.m_vUpVector);
    right.Normalize();

    // 嘗試移動
    Vector3 movement(0, 0, 0);
    if (keyStates['w'] || keyStates['W']) {
        movement = movement + (forward * speed);
    }
    if (keyStates['s'] || keyStates['S']) {
        movement = movement - (forward * speed);
    }
    if (keyStates['a'] || keyStates['A']) {
        movement = movement - (right * speed);
    }
    if (keyStates['d'] || keyStates['D']) {
        movement = movement + (right * speed);
    }

    // 更新位置前先檢查牆壁
    Vector3 newPosition = camera.m_vPosition + movement;

    // 預留一點與牆壁的安全距
    const float WALL_BUFFER = 1.0f;
    bool collision = false;

    // X 軸牆壁
    if (newPosition.x >= WALL_DISTANCE - WALL_BUFFER || newPosition.x <= -WALL_DISTANCE + WALL_BUFFER) {
        collision = true;
    }
    // Z 軸牆壁
    if (newPosition.z >= WALL_DISTANCE - WALL_BUFFER || newPosition.z <= -WALL_DISTANCE + WALL_BUFFER) {
        collision = true;
    }

    if (!collision) {
        // 沒撞牆，更新攝影機位置與 view
        camera.m_vPosition = newPosition;
        camera.m_vView     = camera.m_vView + movement;
    } else {
        // 若撞牆，看是否需要滑動處理；此處簡化為不允許進入
        // 可自行實作「邊滑動邊前進」等邏輯
    }
}

// 檢查籃子或玩家是否接到水果
void checkCollisions() {
    Vector3 cameraPos = camera.m_vPosition;
    const float CATCH_DISTANCE = 1.5f;

    for (auto& fruit : fruits) {
        if (fruit.IsActive()) {
            Vector3 fruitPos = fruit.GetPosition();

            // 先檢查籃子
            float dx = fruitPos.x - basketPosition.x;
            float dz = fruitPos.z - basketPosition.z;
            float distToBasket = sqrt(dx * dx + dz * dz);

            if (distToBasket < BASKET_RADIUS &&
                fruitPos.y < basketPosition.y + BASKET_HEIGHT &&
                fruitPos.y > basketPosition.y) {
                int points = fruit.GetPoints();
                score += points;
                // 若有負分水果則可扣血
                if (points < 0) {
                    life--;
                    if (life <= 0) {
                        currentState = GAMEOVER;
                        gameOverStartTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
                    }
                }
                fruit.SetActive(false);
                continue; // 籃子接到後，就不再判斷玩家
            }

            // 再檢查玩家碰撞
            dx = fruitPos.x - cameraPos.x;
            dz = fruitPos.z - cameraPos.z;
            float distToPlayer = sqrt(dx*dx + dz*dz);
            if (distToPlayer < CATCH_DISTANCE && fruitPos.y < cameraPos.y + 2.0f) {
                int points = fruit.GetPoints();
                score += points;
                if (points < 0) {
                    life--;
                    if (life <= 0) {
                        currentState = GAMEOVER;
                        gameOverStartTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
                    }
                }
                fruit.SetActive(false);
            }
        }
    }
}

// ========================== 場景繪製 ========================== //

// 建立地面與四面牆
void createGroundAndWalls() {
    // 先畫地面（白色或灰色都可，視需求）
    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-GROUND_SIZE, GROUND_Y, -GROUND_SIZE);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( GROUND_SIZE, GROUND_Y, -GROUND_SIZE);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( GROUND_SIZE, GROUND_Y,  GROUND_SIZE);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-GROUND_SIZE, GROUND_Y,  GROUND_SIZE);
    glEnd();

    // 繪製牆壁
    glEnable(GL_TEXTURE_2D);
    wallTexture.BindTexture();
    glColor3f(1.0f, 1.0f, 1.0f);

    // 北牆
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-GROUND_SIZE, GROUND_Y, -WALL_DISTANCE);
        glTexCoord2f(4.0f, 0.0f); glVertex3f( GROUND_SIZE, GROUND_Y, -WALL_DISTANCE);
        glTexCoord2f(4.0f, 1.0f); glVertex3f( GROUND_SIZE, WALL_HEIGHT, -WALL_DISTANCE);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-GROUND_SIZE, WALL_HEIGHT, -WALL_DISTANCE);
    glEnd();

    // 南牆
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-GROUND_SIZE, GROUND_Y, WALL_DISTANCE);
        glTexCoord2f(4.0f, 0.0f); glVertex3f( GROUND_SIZE, GROUND_Y, WALL_DISTANCE);
        glTexCoord2f(4.0f, 1.0f); glVertex3f( GROUND_SIZE, WALL_HEIGHT, WALL_DISTANCE);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-GROUND_SIZE, WALL_HEIGHT, WALL_DISTANCE);
    glEnd();

    // 東牆
    glBegin(GL_QUADS);
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(WALL_DISTANCE, GROUND_Y, -GROUND_SIZE);
        glTexCoord2f(4.0f, 0.0f); glVertex3f(WALL_DISTANCE, GROUND_Y,  GROUND_SIZE);
        glTexCoord2f(4.0f, 1.0f); glVertex3f(WALL_DISTANCE, WALL_HEIGHT,  GROUND_SIZE);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(WALL_DISTANCE, WALL_HEIGHT, -GROUND_SIZE);
    glEnd();

    // 西牆
    glBegin(GL_QUADS);
        glNormal3f(1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-WALL_DISTANCE, GROUND_Y, -GROUND_SIZE);
        glTexCoord2f(4.0f, 0.0f); glVertex3f(-WALL_DISTANCE, GROUND_Y,  GROUND_SIZE);
        glTexCoord2f(4.0f, 1.0f); glVertex3f(-WALL_DISTANCE, WALL_HEIGHT,  GROUND_SIZE);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-WALL_DISTANCE, WALL_HEIGHT, -GROUND_SIZE);
    glEnd();

    wallTexture.UnbindTexture();
    glDisable(GL_TEXTURE_2D);
}

// 使用 code2 的「編織籃子」畫法
/*
void drawBasket() {
    glPushMatrix();
    glTranslatef(basketPosition.x, basketPosition.y, basketPosition.z);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 半透明棕色
    glColor4f(0.8f, 0.4f, 0.0f, 0.6f);

    // --- 繪製編織籃子 ---
    int   numRings       = 3;        // 編織層數
    float ringThickness  = 0.02f;    // 每層間厚度（可自行使用或忽略）
    float initialRadius  = BASKET_RADIUS;
    float radiusStep     = 0.05f;    // 每層半徑縮減量

    // 每層環形
    for (int r = 0; r < numRings; r++) {
        float currentRadius = initialRadius - r * radiusStep;
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < BASKET_SEGMENTS; i++) {
            float angle = (float)i / BASKET_SEGMENTS * 2.0f * M_PI;
            float x = cos(angle) * currentRadius;
            float z = sin(angle) * currentRadius;
            glVertex3f(x, BASKET_HEIGHT, z);
        }
        glEnd();
    }

    // 編織線條
    int numVerticalLines = 12;
    for (int i = 0; i < numVerticalLines; i++) {
        float angle = (float)i / numVerticalLines * 2.0f * M_PI;
        float x_outer = cos(angle) * initialRadius;
        float z_outer = sin(angle) * initialRadius;
        float x_inner = cos(angle) * (initialRadius - (numRings - 1) * radiusStep);
        float z_inner = sin(angle) * (initialRadius - (numRings - 1) * radiusStep);

        glBegin(GL_LINES);
            glVertex3f(x_outer, BASKET_HEIGHT, z_outer);
            glVertex3f(x_inner, 0.0f, z_inner);
        glEnd();
    }

    // 側面
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= BASKET_SEGMENTS; i++) {
        float angle = (float)i / BASKET_SEGMENTS * 2.0f * M_PI;
        float x = cos(angle) * BASKET_RADIUS;
        float z = sin(angle) * BASKET_RADIUS;

        glNormal3f(x, 0.0f, z);
        glVertex3f(x, BASKET_HEIGHT, z);
        glVertex3f(x, 0.0f,        z);
    }
    glEnd();

    // 底部
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    for (int i = 0; i <= BASKET_SEGMENTS; i++) {
        float angle = (float)i / BASKET_SEGMENTS * 2.0f * M_PI;
        float x = cos(angle) * BASKET_RADIUS;
        float z = sin(angle) * BASKET_RADIUS;
        glVertex3f(x, 0.0f, z);
    }
    glEnd();

    // 手把（半圓）
    glColor4f(0.8f, 0.4f, 0.0f, 0.6f);
    glBegin(GL_LINE_STRIP);
    int   handleSegments = 32;
    float handleRadius   = 0.2f;
    float handleY        = BASKET_HEIGHT + 0.1f;
    float handleZ        = BASKET_RADIUS;
    for (int i = 0; i <= handleSegments; i++) {
        float angle = (float)i / handleSegments * M_PI; // 半圓
        float x = cos(angle) * handleRadius;
        float z = sin(angle) * handleRadius;
        glVertex3f(x, handleY, handleZ + z);
    }
    glEnd();

    glDisable(GL_BLEND);
    glPopMatrix();
}

-*/