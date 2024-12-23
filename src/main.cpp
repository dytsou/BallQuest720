// main.cpp
#include <cmath>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <GL/glut.h>
#include "../include/Camera.h"
#include "../include/Fruit.h"
#include "../include/Texture.h"
#include "../include/Text.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace std;

// Global parameters
const int WINDOW_WIDTH  = 1280;
const int WINDOW_HEIGHT = 720;

// Ground and walls
const float GROUND_SIZE = 50.0f;
const float GROUND_Y    = 0.0f;
const float WALL_HEIGHT = 30.0f;
const float WALL_DISTANCE = GROUND_SIZE;
CTexture wallTexture;

// Camera
CCamera camera;

// HUD & Score
Text    scoreText;
int     score = 0;
int     life  = 20;

// Fruit related
const int  BallHeight = 50;
const int  Interval   = 20;

// Basket
const float BASKET_RADIUS   = 0.1f;
const float BASKET_HEIGHT   = 0.1f;
const int   BASKET_SEGMENTS = 32;
Vector3 basketPosition(0.0f, 1.0f, 0.0f);

// Ring and scoring area
const float CATCH_DISTANCE = 0.8f;
const float RING_RADIUS = CATCH_DISTANCE;
const int   RING_SEGMENTS = 50;

// Mouse and keyboard input
bool keyStates[256] = {false};
bool specialKeyStates[256] = {false};
int  lastMouseX = WINDOW_WIDTH / 2;
int  lastMouseY = WINDOW_HEIGHT / 2;
bool firstMouse = true;
float yaw   = -90.0f;
float pitch = 0.0f;

// Game state & difficulty
enum GameState {
    MENU,
    PLAYING,
    GAMEOVER
};
enum Difficulty {
    EASY,
    MEDIUM,
    HARD
};

// Menu button structure
struct Button {
    float x, y, width, height;
    string text;
    bool isHovered;
};

// Menu buttons
vector<Button> difficultyButtons = {
    {WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 - 80, 200, 50, "Easy",   false},
    {WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2,      200, 50, "Medium", false},
    {WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 + 80, 200, 50, "Hard",   false}
};

GameState   currentState        = MENU;
Difficulty  selectedDifficulty  = MEDIUM;

// Game timing and parameters
float gameTime          = 0.0f;
const float GAME_DURATION = 120.0f;
float gameOverStartTime = 0.0f;

// Speed and sensitivity parameters
float cameraSpeed         = 0.1f;
float mouseSensitivity    = 0.05f;
float fruitSpeedMultiplier = 1.0f;

// Explosion effect
float explosionTime = 0.0f;
bool isExploding = false;
const float EXPLOSION_DURATION = 2.0f;

// Function declarations
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
void drawRing();

// Global fruit containers
vector<Fruit> mainFruits;
vector<Fruit> blackFruits;

// Initialize fruits with default positions
void InitializeFruits() {
    // Generate initial main fruits
    for (int i = 0; i < 5; ++i) {
        Vector3 pos(0.0f, 0.0f, 0.0f);
        mainFruits.emplace_back(pos, FruitType::MAIN);
    }
    // Generate initial black balls
    for (int i = 0; i < 3; ++i) {
        Vector3 pos(0.0f, 0.0f, 0.0f);
        blackFruits.emplace_back(pos, FruitType::BLACK);
    }
}

void initializeGLUT(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("BallQuest 720");
}

void setupCallbacks() {
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);
    glutPassiveMotionFunc(passiveMotion);
    glutIdleFunc(update);
}

int main(int argc, char** argv) {
    // Initialize GLUT and create window
    initializeGLUT(argc, argv);

    // Initialize OpenGL settings and game state
    init();

    // Initialize game objects
    InitializeFruits();

    // Set up callback functions
    setupCallbacks();

    // Show cursor in menu
    glutSetCursor(GLUT_CURSOR_LEFT_ARROW);

    // Start game loop
    glutMainLoop();
    
    return 0;
}

void init() {
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    wallTexture.LoadTexture("../textures/wall.bmp");

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    GLfloat lightPosition[] = { 0.0f, 10.0f, 0.0f, 1.0f };
    GLfloat lightAmbient[]  = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat lightDiffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiffuse);

    camera.PositionCamera(
        0.0f, 2.0f, 6.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    );
    
    srand(static_cast<unsigned>(time(nullptr)));

    gameTime          = 0.0f;
    gameOverStartTime = 0.0f;
}

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
        drawMenu();
        return;
    }
    else if (currentState == GAMEOVER) {
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 0.0f, 0.0f);
        string gameOverText    = "Game Over";
        string finalScoreText  = "Final Score: " + to_string(score);

        scoreText.RenderText(WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2,     gameOverText);
        glColor3f(1.0f, 1.0f, 1.0f);
        scoreText.RenderText(WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2 - 40, finalScoreText);

        glutSwapBuffers();
        return;
    }

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    camera.Look();

    createGroundAndWalls();
    drawRing();

    for (auto& fruit : mainFruits) {
        fruit.Draw();
    }

    for (auto& fruit : blackFruits) {
        fruit.Draw();
    }

    glDisable(GL_LIGHTING);
    glColor3f(0.0f, 0.0f, 0.0f);

    stringstream ss;
    ss << "Score: " << score << "  Life: " << life;
    scoreText.RenderText(10, 30, ss.str());

    float remainingTime = GAME_DURATION - gameTime;
    if (remainingTime < 0.0f) remainingTime = 0.0f;
    stringstream timeSS;
    timeSS << fixed << setprecision(1) << "Time: " << remainingTime << " sec";
    scoreText.RenderText(10, 60, timeSS.str());

    glEnable(GL_LIGHTING);

    if (isExploding) {
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        
        float alpha = 1.0f - (explosionTime / EXPLOSION_DURATION);
        if (alpha < 0.0f) alpha = 0.0f;
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, alpha);
        
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        glBegin(GL_QUADS);
            glVertex2f(0, 0);
            glVertex2f(WINDOW_WIDTH, 0);
            glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
            glVertex2f(0, WINDOW_HEIGHT);
        glEnd();
        
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
    }

    glutSwapBuffers();
}

void update() {
    static float lastTime = 0.0f;
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; // 轉秒
    float deltaTime   = currentTime - lastTime;
    lastTime          = currentTime;

    if (currentState == MENU || currentState == GAMEOVER) {
        glutPostRedisplay();
        return;
    }

    if (isExploding) {
        explosionTime += deltaTime;
        if (explosionTime >= EXPLOSION_DURATION) {
            isExploding = false;
            explosionTime = 0.0f;
        }
    }

    gameTime += deltaTime;
    if (gameTime >= GAME_DURATION) {
        currentState       = GAMEOVER;
        gameOverStartTime  = currentTime;
        return;
    }

    processKeys();

    Vector3 forward = camera.m_vView - camera.m_vPosition;
    forward.y = 0;
    forward.Normalize();
    forward = forward * 0.7f;

    basketPosition.x = camera.m_vPosition.x + forward.x;
    basketPosition.z = camera.m_vPosition.z + forward.z;
    basketPosition.y = camera.m_vPosition.y + 0.1f;

    for (auto& fruit : mainFruits) {
        fruit.Update(deltaTime * fruitSpeedMultiplier); 
    }

    for (auto& fruit : blackFruits) {
        fruit.Update(deltaTime * fruitSpeedMultiplier); 
    }

    checkCollisions();

    for (auto& fruit : mainFruits) {
        if (!fruit.IsActive()) {
            fruit.ResetRandomFruit(BallHeight, gameTime, FruitType::MAIN); 
        }
    }

    for (auto& fruit : blackFruits) {
        if (!fruit.IsActive()) {
            fruit.ResetRandomFruit(BallHeight, gameTime, FruitType::BLACK); 
        }
    }

    glutPostRedisplay();
}


void drawMenu() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Set background color
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Title and instruction text
    const int charWidth = 15;
    string titleText    = "Select Difficulty";
    string instructText1= "Use WASD to move, Mouse to look around";
    string instructText2= "Press Q to quit game";
    string instructText3= "Press Z to end game and show score";

    int titleWidth   = titleText.size()    * charWidth;
    int instruct1Width = instructText1.size() * charWidth;
    int instruct2Width = instructText2.size() * charWidth;
    int instruct3Width = instructText3.size() * charWidth;

    glDisable(GL_LIGHTING);
    glColor3f(0.9f, 0.9f, 0.9f);  // Title color
    scoreText.RenderText(WINDOW_WIDTH/2 - titleWidth/2 + 55, WINDOW_HEIGHT/3 - 10, titleText);
    for (const auto& btn : difficultyButtons) {
        drawButton(btn);
    }

    // Draw instruction text (yellow)
    glColor3f(1.0f, 1.0f, 0.0f);
    scoreText.RenderText(WINDOW_WIDTH/2 - instruct1Width/2 + 90, WINDOW_HEIGHT - 130, instructText1);
    scoreText.RenderText(WINDOW_WIDTH/2 - instruct2Width/2 + 60, WINDOW_HEIGHT - 100, instructText2);
    scoreText.RenderText(WINDOW_WIDTH/2 - instruct3Width/2 + 90, WINDOW_HEIGHT - 70,  instructText3);

    glutSwapBuffers();
}

void drawButton(const Button& btn) {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    if (btn.isHovered) {
        glColor4f(0.4f, 0.7f, 1.0f, 1.0f);  // Light blue when hovered
    } else {
        glColor4f(0.1f, 0.3f, 0.6f, 1.0f);  // Dark blue normally
    }

    // Button background
    glBegin(GL_QUADS);
        glVertex2f(btn.x,            btn.y);
        glVertex2f(btn.x+btn.width,  btn.y);
        glVertex2f(btn.x+btn.width,  btn.y+btn.height);
        glVertex2f(btn.x,            btn.y+btn.height);
    glEnd();

    // Button border (white)
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(btn.x,            btn.y);
        glVertex2f(btn.x+btn.width,  btn.y);
        glVertex2f(btn.x+btn.width,  btn.y+btn.height);
        glVertex2f(btn.x,            btn.y+btn.height);
    glEnd();

    // Button text
    glColor3f(1.0f, 1.0f, 1.0f);
    float textX = btn.x + (btn.width - btn.text.length() * 9) / 2.0f;
    float textY = btn.y + (btn.height + 10) / 2.0f;
    scoreText.RenderText(textX, textY, btn.text);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void startGame(Difficulty diff) {
    currentState       = PLAYING;
    selectedDifficulty = diff;
    score             = 0;
    gameTime          = 0.0f;
    gameOverStartTime = 0.0f;

    mainFruits.clear();
    blackFruits.clear();

    switch (diff) {
        case EASY:
            life = 5;
            fruitSpeedMultiplier = 1.0f;
            for (int i = 0; i < 5; ++i) {
                mainFruits.emplace_back(Vector3(0, BallHeight + 5*i, 0), FruitType::MAIN);
            }
            for (int i = 0; i < 3; ++i) {
                blackFruits.emplace_back(Vector3(0, BallHeight + 5*i, 0), FruitType::BLACK);
            }
            break;
        case MEDIUM:
            life = 3;
            fruitSpeedMultiplier = 1.5f;
            for (int i = 0; i < 7; ++i) {
                mainFruits.emplace_back(Vector3(0, BallHeight + 5*i, 0), FruitType::MAIN);
            }
            for (int i = 0; i < 5; ++i) {
                blackFruits.emplace_back(Vector3(0, BallHeight + 5*i, 0), FruitType::BLACK);
            }
            break;
        case HARD:
            life = 1;
            fruitSpeedMultiplier = 2.0f;
            for (int i = 0; i < 10; ++i) {
                mainFruits.emplace_back(Vector3(0, BallHeight + 5*i, 0), FruitType::MAIN);
            }
            for (int i = 0; i < 7; ++i) {
                blackFruits.emplace_back(Vector3(0, BallHeight + 5*i, 0), FruitType::BLACK);
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


void mouse(int button, int state, int x, int y) {
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
        for (auto& btn : difficultyButtons) {
            btn.isHovered = 
                (x >= btn.x && x <= btn.x + btn.width &&
                 y >= btn.y && y <= btn.y + btn.height);
        }
        glutPostRedisplay();
    }
    else if (currentState == PLAYING) {
        mouseMotion(x, y);
    }
}

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

    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    Vector3 direction;
    direction.x = cos(yaw * 0.0174532925f) * cos(pitch * 0.0174532925f);
    direction.y = sin(pitch * 0.0174532925f);
    direction.z = sin(yaw * 0.0174532925f) * cos(pitch * 0.0174532925f);
    direction.Normalize();

    camera.m_vView = camera.m_vPosition + direction;

    if (x != WINDOW_WIDTH/2 || y != WINDOW_HEIGHT/2) {
        glutWarpPointer(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
        lastMouseX = WINDOW_WIDTH/2;
        lastMouseY = WINDOW_HEIGHT/2;
    }
}

void specialKeys(int key, int x, int y) {
    // for special keys
}

// keyboard down
void keyboard(unsigned char key, int x, int y) {
    keyStates[key] = true;

    if (key == 27 || key == 'q' || key == 'Q') {
        exit(0);
    }

    if (currentState == PLAYING) {
        if (key == 'z' || key == 'Z') {
            currentState = GAMEOVER;
            gameOverStartTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
            return;
        }

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

// keyboard up
void keyboardUp(unsigned char key, int x, int y) {
    keyStates[key] = false;
}

void processKeys() {
    if (currentState != PLAYING) return;

    float speed = cameraSpeed;
    if (keyStates[' ']) {
        speed *= 2.0f;
    }

    Vector3 forward = camera.m_vView - camera.m_vPosition;
    forward.y = 0;
    forward.Normalize();

    Vector3 right = forward.Cross(camera.m_vUpVector);
    right.Normalize();

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

    Vector3 newPosition = camera.m_vPosition + movement;

    const float WALL_BUFFER = 1.0f;
    bool collision = false;

    if (newPosition.x >= WALL_DISTANCE - WALL_BUFFER || newPosition.x <= -WALL_DISTANCE + WALL_BUFFER) {
        collision = true;
    }
    if (newPosition.z >= WALL_DISTANCE - WALL_BUFFER || newPosition.z <= -WALL_DISTANCE + WALL_BUFFER) {
        collision = true;
    }

    if (!collision) {
        camera.m_vPosition = newPosition;
        camera.m_vView     = camera.m_vView + movement;
    }
}

void checkCollisions() {
    Vector3 cameraPos = camera.m_vPosition;
    
    Vector3 viewDir = camera.m_vView - camera.m_vPosition;
    viewDir.Normalize();
    Vector3 ringPos = camera.m_vPosition + (viewDir * 2.0f);

    for (auto& fruit : mainFruits) {
        if (fruit.IsActive()) {
            Vector3 fruitPos = fruit.GetPosition();
            
            Vector3 toFruit = fruitPos - ringPos;
            float distAlongView = toFruit.Dot(viewDir);
            Vector3 projection = ringPos + viewDir * distAlongView;
            Vector3 toAxis = fruitPos - projection;
            float distToAxis = sqrt(toAxis.x * toAxis.x + toAxis.y * toAxis.y + toAxis.z * toAxis.z);
            
            static Vector3 lastFruitPos = fruitPos;
            float lastDistAlongView = (lastFruitPos - ringPos).Dot(viewDir);
            
            if ((lastDistAlongView * distAlongView < 0) &&
                (distToAxis <= RING_RADIUS) &&
                (distToAxis >= RING_RADIUS * 0.8f)) {
                
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
                continue;
            }
            
            lastFruitPos = fruitPos;
            
            float dx = fruitPos.x - cameraPos.x;
            float dz = fruitPos.z - cameraPos.z;
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

    for (auto& fruit : blackFruits) {
        if (fruit.IsActive()) {
            Vector3 fruitPos = fruit.GetPosition();
            
            Vector3 toFruit = fruitPos - ringPos;
            float distAlongView = toFruit.Dot(viewDir);
            Vector3 projection = ringPos + viewDir * distAlongView;
            Vector3 toAxis = fruitPos - projection;
            float distToAxis = sqrt(toAxis.x * toAxis.x + toAxis.y * toAxis.y + toAxis.z * toAxis.z);
            
            static Vector3 lastFruitPos = fruitPos;
            float lastDistAlongView = (lastFruitPos - ringPos).Dot(viewDir);
            
            if ((lastDistAlongView * distAlongView < 0) &&
                (distToAxis <= RING_RADIUS) &&
                (distToAxis >= RING_RADIUS * 0.8f)) {
                
                isExploding = true;
                explosionTime = 0.0f;
                
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
                continue;
            }
            
            lastFruitPos = fruitPos;
            
            float dx = fruitPos.x - cameraPos.x;
            float dz = fruitPos.z - cameraPos.z;
            float distToPlayer = sqrt(dx*dx + dz*dz);
            if (distToPlayer < CATCH_DISTANCE && fruitPos.y < cameraPos.y + 2.0f) {
                isExploding = true;
                explosionTime = 0.0f;
                
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

void createGroundAndWalls() {
    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-GROUND_SIZE, GROUND_Y, -GROUND_SIZE);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( GROUND_SIZE, GROUND_Y, -GROUND_SIZE);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( GROUND_SIZE, GROUND_Y,  GROUND_SIZE);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-GROUND_SIZE, GROUND_Y,  GROUND_SIZE);
    glEnd();

    glEnable(GL_TEXTURE_2D);
    wallTexture.BindTexture();
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-GROUND_SIZE, GROUND_Y, -WALL_DISTANCE);
        glTexCoord2f(4.0f, 0.0f); glVertex3f( GROUND_SIZE, GROUND_Y, -WALL_DISTANCE);
        glTexCoord2f(4.0f, 1.0f); glVertex3f( GROUND_SIZE, WALL_HEIGHT, -WALL_DISTANCE);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-GROUND_SIZE, WALL_HEIGHT, -WALL_DISTANCE);
    glEnd();

    glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-GROUND_SIZE, GROUND_Y, WALL_DISTANCE);
        glTexCoord2f(4.0f, 0.0f); glVertex3f( GROUND_SIZE, GROUND_Y, WALL_DISTANCE);
        glTexCoord2f(4.0f, 1.0f); glVertex3f( GROUND_SIZE, WALL_HEIGHT, WALL_DISTANCE);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-GROUND_SIZE, WALL_HEIGHT, WALL_DISTANCE);
    glEnd();

    glBegin(GL_QUADS);
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(WALL_DISTANCE, GROUND_Y, -GROUND_SIZE);
        glTexCoord2f(4.0f, 0.0f); glVertex3f(WALL_DISTANCE, GROUND_Y,  GROUND_SIZE);
        glTexCoord2f(4.0f, 1.0f); glVertex3f(WALL_DISTANCE, WALL_HEIGHT,  GROUND_SIZE);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(WALL_DISTANCE, WALL_HEIGHT, -GROUND_SIZE);
    glEnd();

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

void drawRing() {
    glDisable(GL_LIGHTING);
    
    glPushMatrix();
    
    Vector3 viewDir = camera.m_vView - camera.m_vPosition;
    viewDir.Normalize();
    Vector3 ringPos = camera.m_vPosition + (viewDir * 2.0f);
    
    glTranslatef(ringPos.x, ringPos.y, ringPos.z);
    
    float angleY = atan2(viewDir.z, viewDir.x) * 180.0f / M_PI;
    float angleX = atan2(viewDir.y, sqrt(viewDir.x * viewDir.x + viewDir.z * viewDir.z)) * 180.0f / M_PI;
    
    glRotatef(-angleY, 0, 1, 0);
    glRotatef(angleX, 1, 0, 0);
    
    glLineWidth(5.0f);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    const float innerRadius = RING_RADIUS * 0.8f;
    float alpha = 0.3f;
    
    glColor4f(1.0f, 1.0f, 0.0f, alpha);
    
    glBegin(GL_QUAD_STRIP);
    for(int i = 0; i <= RING_SEGMENTS; i++) {
        float theta = 2.0f * M_PI * float(i) / float(RING_SEGMENTS);
        float x1 = innerRadius * cosf(theta);
        float y1 = innerRadius * sinf(theta);
        float x2 = RING_RADIUS * cosf(theta);
        float y2 = RING_RADIUS * sinf(theta);
        
        glVertex3f(x1, y1, 0);
        glVertex3f(x2, y2, 0);
    }
    glEnd();
    
    glColor4f(1.0f, 1.0f, 0.0f, alpha + 0.2f);
    
    glBegin(GL_LINE_LOOP);
    for(int i = 0; i < RING_SEGMENTS; i++) {
        float theta = 2.0f * M_PI * float(i) / float(RING_SEGMENTS);
        float x = RING_RADIUS * cosf(theta);
        float y = RING_RADIUS * sinf(theta);
        glVertex3f(x, y, 0);
    }
    glEnd();
    
    glBegin(GL_LINE_LOOP);
    for(int i = 0; i < RING_SEGMENTS; i++) {
        float theta = 2.0f * M_PI * float(i) / float(RING_SEGMENTS);
        float x = innerRadius * cosf(theta);
        float y = innerRadius * sinf(theta);
        glVertex3f(x, y, 0);
    }
    glEnd();
    
    glLineWidth(1.0f);
    glDisable(GL_BLEND);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}
