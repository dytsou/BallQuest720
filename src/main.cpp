#include <cmath>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <GL/glut.h>
#include "../include/Camera.h"
#include "../include/Fruit.h"
#include "../include/Texture.h"
#include "../include/Text.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace std;

// Global variables
CCamera camera;
Text scoreText;
int score = 0;
int life = 20;
const int BallHeight = 50;
const int Interval = 20;
vector<Fruit> fruits;

// Ground texture
GLuint groundTexture;

// Window dimensions
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

// Ground vertices
const float GROUND_SIZE = 50.0f;
const float GROUND_Y = 0.0f;

// Movement flags
bool keyStates[256] = {false};
bool specialKeyStates[256] = {false};
int lastMouseX = WINDOW_WIDTH/2;
int lastMouseY = WINDOW_HEIGHT/2;
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;

// Add basket constants after other global variables
const float BASKET_RADIUS = 0.5f;
const float BASKET_HEIGHT = 0.4f;
const int BASKET_SEGMENTS = 32;
Vector3 basketPosition(0.0f, 1.0f, 0.0f);  // Position of the basket

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
void createGround();
void checkCollisions();
void processKeys();
void drawBasket();

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Ball Catcher Game");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);
    glutPassiveMotionFunc(mouseMotion);
    glutIdleFunc(update);

    // Hide cursor and center it
    glutSetCursor(GLUT_CURSOR_NONE);
    glutWarpPointer(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);

    glutMainLoop();
    return 0;
}

void init() {
    glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    
    // Enable lighting after setting up the window
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Set up lighting
    GLfloat lightPosition[] = { 0.0f, 10.0f, 0.0f, 1.0f };
    GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

    // Initialize camera
    camera.PositionCamera(0.0f, 2.0f, 6.0f,   // Position
                         0.0f, 0.0f, 0.0f,   // View
                         0.0f, 1.0f, 0.0f);  // Up vector

    // Initialize random seed
    srand(time(NULL));

    // Initialize fruits
    for (int i = 0; i < 5; ++i) {
        fruits.push_back(Fruit(Vector3(0, BallHeight + 15 * i, 0)));
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Set up camera
    camera.Look();

    // Draw ground with solid color
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.5f, 0.5f, 0.5f);  // Gray color for ground
    createGround();

    // Draw basket
    drawBasket();

    // Draw fruits
    for (auto& fruit : fruits) {
        fruit.Draw();
    }

    // Draw score and life
    glDisable(GL_LIGHTING);
    stringstream ss;
    ss << "Score: " << score << " Life: " << life;
    scoreText.RenderText(10, 30, ss.str());
    glEnable(GL_LIGHTING);

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    float ratio = (float)w / h;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, ratio, 0.1f, 1000.0f);
    glMatrixMode(GL_MODELVIEW);
}

void update() {
    static float lastTime = 0.0f;
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    // Process keyboard input
    processKeys();

    // Update basket position to follow player
    Vector3 forward = camera.m_vView - camera.m_vPosition;
    forward.y = 0;  // Keep it in horizontal plane
    forward.Normalize();
    forward = forward * 0.7f;  // Place basket 0.7 units in front of player
    
    // Update basket position
    basketPosition.x = camera.m_vPosition.x + forward.x;
    basketPosition.z = camera.m_vPosition.z + forward.z;
    basketPosition.y = camera.m_vPosition.y + 0.1f;  // Place basket slightly above eye level

    // Update fruits
    for (auto& fruit : fruits) {
        fruit.Update(deltaTime);
    }

    // Check collisions
    checkCollisions();

    // Reset inactive fruits
    for (auto& fruit : fruits) {
        if (!fruit.IsActive()) {
            fruit.ResetRandomFruit(BallHeight);
        }
    }

    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    keyStates[key] = true;
    
    if (key == 'q' || key == 'Q') {
        exit(0);
    }
    else if (key == 'z' || key == 'Z') {
        cout << "Final Score: " << score << endl;
        exit(0);
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    keyStates[key] = false;
}

void processKeys() {
    float speed = 0.5f;
    if (keyStates[' ']) { // Space bar for sprint
        speed = 1.0f;
    }

    Vector3 forward = camera.m_vView - camera.m_vPosition;
    forward.y = 0; // Keep movement in horizontal plane
    forward.Normalize();

    Vector3 right = forward.Cross(camera.m_vUpVector);
    right.Normalize();

    if (keyStates['w'] || keyStates['W']) {
        camera.m_vPosition = camera.m_vPosition + (forward * speed);
        camera.m_vView = camera.m_vView + (forward * speed);
    }
    if (keyStates['s'] || keyStates['S']) {
        camera.m_vPosition = camera.m_vPosition - (forward * speed);
        camera.m_vView = camera.m_vView - (forward * speed);
    }
    if (keyStates['a'] || keyStates['A']) {
        camera.m_vPosition = camera.m_vPosition - (right * speed);
        camera.m_vView = camera.m_vView - (right * speed);
    }
    if (keyStates['d'] || keyStates['D']) {
        camera.m_vPosition = camera.m_vPosition + (right * speed);
        camera.m_vView = camera.m_vView + (right * speed);
    }
}

void mouseMotion(int x, int y) {
    if (firstMouse) {
        lastMouseX = x;
        lastMouseY = y;
        firstMouse = false;
    }

    float xoffset = x - lastMouseX;
    float yoffset = lastMouseY - y;
    lastMouseX = x;
    lastMouseY = y;

    float sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Limit pitch to avoid flipping
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    // Calculate new view direction
    Vector3 direction;
    direction.x = cos(yaw * 0.0174532925f) * cos(pitch * 0.0174532925f);
    direction.y = sin(pitch * 0.0174532925f);
    direction.z = sin(yaw * 0.0174532925f) * cos(pitch * 0.0174532925f);
    direction.Normalize();

    camera.m_vView = camera.m_vPosition + direction;

    // Keep cursor in window center
    if (x != WINDOW_WIDTH/2 || y != WINDOW_HEIGHT/2) {
        glutWarpPointer(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
        lastMouseX = WINDOW_WIDTH/2;
        lastMouseY = WINDOW_HEIGHT/2;
    }
}

void specialKeys(int key, int x, int y) {
    // Currently not used
}

void mouse(int button, int state, int x, int y) {
    // Currently not used
}

void createGround() {
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-GROUND_SIZE, GROUND_Y, -GROUND_SIZE);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( GROUND_SIZE, GROUND_Y, -GROUND_SIZE);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( GROUND_SIZE, GROUND_Y,  GROUND_SIZE);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-GROUND_SIZE, GROUND_Y,  GROUND_SIZE);
    glEnd();
}

void checkCollisions() {
    Vector3 cameraPos = camera.m_vPosition;
    const float CATCH_DISTANCE = 1.5f;

    for (auto& fruit : fruits) {
        if (fruit.IsActive()) {
            Vector3 fruitPos = fruit.GetPosition();
            
            // Check if fruit is above basket
            float dx = fruitPos.x - basketPosition.x;
            float dz = fruitPos.z - basketPosition.z;
            float distanceToBasket = sqrt(dx * dx + dz * dz);
            
            // If fruit is within basket radius and at right height
            if (distanceToBasket < BASKET_RADIUS && 
                fruitPos.y < basketPosition.y + BASKET_HEIGHT && 
                fruitPos.y > basketPosition.y) {
                int points = fruit.GetPoints();
                score += points;  // Add points based on fruit type
                if (points < 0) {
                    life--;
                    if (life <= 0) {
                        cout << "Game Over! Final Score: " << score << endl;
                        exit(0);
                    }
                }
                fruit.SetActive(false);
                continue;  // Skip player collision check for caught fruits
            }
            
            // Check player collision
            dx = fruitPos.x - cameraPos.x;
            dz = fruitPos.z - cameraPos.z;
            float distance = sqrt(dx * dx + dz * dz);

            if (distance < CATCH_DISTANCE && fruitPos.y < cameraPos.y + 2.0f) {
                int points = fruit.GetPoints();
                score += points;
                if (points < 0) {
                    life--;
                    if (life <= 0) {
                        cout << "Game Over! Final Score: " << score << endl;
                        exit(0);
                    }
                }
                fruit.SetActive(false);
            }
        }
    }
}

void drawBasket() {
    glPushMatrix();
    glTranslatef(basketPosition.x, basketPosition.y, basketPosition.z);
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Set material properties for the basket - semi-transparent brown
    glColor4f(0.8f, 0.4f, 0.0f, 0.6f);  // Brown color with 60% opacity
    
    // Draw the basket rim
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= BASKET_SEGMENTS; i++) {
        float angle = (float)i / BASKET_SEGMENTS * 2.0f * M_PI;
        float x = cos(angle) * BASKET_RADIUS;
        float z = sin(angle) * BASKET_RADIUS;
        
        // Outer vertex
        glNormal3f(x, 0.0f, z);
        glVertex3f(x, BASKET_HEIGHT, z);
        
        // Inner vertex
        glVertex3f(x * 0.8f, BASKET_HEIGHT, z * 0.8f);
    }
    glEnd();
    
    // Draw the basket sides
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= BASKET_SEGMENTS; i++) {
        float angle = (float)i / BASKET_SEGMENTS * 2.0f * M_PI;
        float x = cos(angle) * BASKET_RADIUS;
        float z = sin(angle) * BASKET_RADIUS;
        
        glNormal3f(x, 0.0f, z);
        glVertex3f(x, BASKET_HEIGHT, z);
        glVertex3f(x, 0.0f, z);
    }
    glEnd();
    
    // Draw the bottom
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);  // Center point
    for (int i = 0; i <= BASKET_SEGMENTS; i++) {
        float angle = (float)i / BASKET_SEGMENTS * 2.0f * M_PI;
        float x = cos(angle) * BASKET_RADIUS;
        float z = sin(angle) * BASKET_RADIUS;
        glVertex3f(x, 0.0f, z);
    }
    glEnd();
    
    glDisable(GL_BLEND);  // Disable blending after drawing the basket
    glPopMatrix();
} 