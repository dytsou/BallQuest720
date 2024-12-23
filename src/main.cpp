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

// Game states
enum GameState {
    MENU,
    PLAYING
};

// Difficulty levels
enum Difficulty {
    EASY,
    NORMAL,
    HARD
};

// Global variables
GameState currentState = MENU;
Difficulty selectedDifficulty = NORMAL;
CCamera camera;
Text scoreText;
int score = 0;
int life = 20;
const int BallHeight = 50;
const int Interval = 20;
vector<Fruit> fruits;

// Button dimensions
struct Button {
    float x, y, width, height;
    string text;
    bool isHovered;
};

vector<Button> difficultyButtons = {
    {WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 - 80, 200, 50, "Easy", false},
    {WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2, 200, 50, "Normal", false},
    {WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 + 80, 200, 50, "Hard", false}
};

// Add wall constants after ground constants
const float WALL_HEIGHT = 10.0f;
const float WALL_DISTANCE = GROUND_SIZE;  // Place walls at the edge of the ground

// Add texture object after other global variables
CTexture wallTexture;

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
void createGround();
void checkCollisions();
void processKeys();
void drawBasket();
void drawMenu();
void drawButton(const Button& btn);
void startGame(Difficulty diff);

// Function to start the game with selected difficulty
void startGame(Difficulty diff) {
    currentState = PLAYING;
    selectedDifficulty = diff;
    score = 0;
    
    // Set difficulty-specific parameters
    switch(diff) {
        case EASY:
            life = 30;
            fruits.clear();
            for (int i = 0; i < 3; ++i) {
                fruits.push_back(Fruit(Vector3(0, BallHeight + 15 * i, 0)));
            }
            break;
        case NORMAL:
            life = 20;
            fruits.clear();
            for (int i = 0; i < 5; ++i) {
                fruits.push_back(Fruit(Vector3(0, BallHeight + 15 * i, 0)));
            }
            break;
        case HARD:
            life = 10;
            fruits.clear();
            for (int i = 0; i < 7; ++i) {
                fruits.push_back(Fruit(Vector3(0, BallHeight + 15 * i, 0)));
            }
            break;
    }
    
    // Reset camera position
    camera.PositionCamera(0.0f, 2.0f, 6.0f,   // Position
                         0.0f, 0.0f, 0.0f,   // View
                         0.0f, 1.0f, 0.0f);  // Up vector
    
    // Hide cursor and center it
    glutSetCursor(GLUT_CURSOR_NONE);
    glutWarpPointer(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
    firstMouse = true;
}

// Function to draw a button
void drawButton(const Button& btn) {
    // Draw button background
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    if (btn.isHovered) {
        glColor4f(0.4f, 0.7f, 1.0f, 1.0f);  // Brighter blue when highlighted
    } else {
        glColor4f(0.1f, 0.3f, 0.6f, 1.0f);  // Darker blue for normal state
    }
    
    // Draw button with a border
    glBegin(GL_QUADS);
    glVertex2f(btn.x, btn.y);
    glVertex2f(btn.x + btn.width, btn.y);
    glVertex2f(btn.x + btn.width, btn.y + btn.height);
    glVertex2f(btn.x, btn.y + btn.height);
    glEnd();
    
    // Draw button border
    glColor3f(1.0f, 1.0f, 1.0f);  // White border
    glBegin(GL_LINE_LOOP);
    glVertex2f(btn.x, btn.y);
    glVertex2f(btn.x + btn.width, btn.y);
    glVertex2f(btn.x + btn.width, btn.y + btn.height);
    glVertex2f(btn.x, btn.y + btn.height);
    glEnd();
    
    // Draw button text
    glColor3f(1.0f, 1.0f, 1.0f);  // White text
    float textX = btn.x + (btn.width - btn.text.length() * 9) / 2;  // Center text
    float textY = btn.y + (btn.height + 10) / 2;
    scoreText.RenderText(textX, textY, btn.text);
    
    glEnable(GL_DEPTH_TEST);
}

// Function to draw the menu
void drawMenu() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    // Set darker background color
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Calculate text widths (assuming each character is roughly 15 pixels wide)
    const int charWidth = 15;
    string titleText = "Select Difficulty";
    string instructText1 = "Use WASD to move, Mouse to look around";
    string instructText2 = "Press Q to quit game";
    
    int titleWidth = titleText.length() * charWidth;
    int instruct1Width = instructText1.length() * charWidth;
    int instruct2Width = instructText2.length() * charWidth;
    
    // Draw title (centered)
    glColor3f(0.9f, 0.9f, 0.9f);  // Light gray for title
    scoreText.RenderText(WINDOW_WIDTH/2 - titleWidth/2 + 55, WINDOW_HEIGHT/3 - 10, titleText);
    
    // Draw buttons
    for (const auto& btn : difficultyButtons) {
        drawButton(btn);
    }
    
    // Draw instructions (centered)
    glColor3f(1.0f, 1.0f, 0.0f);  // Yellow color for instructions
    scoreText.RenderText(WINDOW_WIDTH/2 - instruct1Width/2 + 90, WINDOW_HEIGHT - 100, instructText1);
    scoreText.RenderText(WINDOW_WIDTH/2 - instruct2Width/2 + 60, WINDOW_HEIGHT - 70, instructText2);
    
    glutSwapBuffers();
}

void display() {
    if (currentState == MENU) {
        drawMenu();
        return;
    }
    
    // Set white background for game
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    camera.Look();

    glDisable(GL_TEXTURE_2D);
    createGround();
    drawBasket();

    for (auto& fruit : fruits) {
        fruit.Draw();
    }

    glDisable(GL_LIGHTING);
    stringstream ss;
    ss << "Score: " << score << " Life: " << life;
    glColor3f(0.0f, 0.0f, 0.0f);  // Black color for text
    scoreText.RenderText(10, 30, ss.str());
    glEnable(GL_LIGHTING);

    glutSwapBuffers();
}

void mouse(int button, int state, int x, int y) {
    if (currentState == MENU && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Check if any button was clicked
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
        // Update button hover states
        for (auto& btn : difficultyButtons) {
            btn.isHovered = (x >= btn.x && x <= btn.x + btn.width &&
                            y >= btn.y && y <= btn.y + btn.height);
        }
        glutPostRedisplay();
    } else {
        mouseMotion(x, y);
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("BallQuest 720");  // Updated window title

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);
    glutPassiveMotionFunc(passiveMotion);
    glutIdleFunc(update);

    // Show cursor for menu
    glutSetCursor(GLUT_CURSOR_LEFT_ARROW);

    glutMainLoop();
    return 0;
}

void init() {
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    
    // Load wall texture with corrected path
    wallTexture.LoadTexture("../textures/wall.bmp");
    
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
    if (currentState == MENU) {
        glutPostRedisplay();
        return;
    }

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

void createGround() {
    // Set white color for ground
    glColor3f(1.0f, 1.0f, 1.0f);
    
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-GROUND_SIZE, GROUND_Y, -GROUND_SIZE);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( GROUND_SIZE, GROUND_Y, -GROUND_SIZE);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( GROUND_SIZE, GROUND_Y,  GROUND_SIZE);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-GROUND_SIZE, GROUND_Y,  GROUND_SIZE);
    glEnd();
    
    // Draw reference walls with texture
    glColor3f(1.0f, 1.0f, 1.0f);  // White color to show texture properly
    glEnable(GL_TEXTURE_2D);
    wallTexture.BindTexture();  // Enable wall texture
    
    // North wall
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-GROUND_SIZE, GROUND_Y, -WALL_DISTANCE);
    glTexCoord2f(4.0f, 0.0f); glVertex3f( GROUND_SIZE, GROUND_Y, -WALL_DISTANCE);
    glTexCoord2f(4.0f, 1.0f); glVertex3f( GROUND_SIZE, WALL_HEIGHT, -WALL_DISTANCE);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-GROUND_SIZE, WALL_HEIGHT, -WALL_DISTANCE);
    glEnd();
    
    // South wall
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-GROUND_SIZE, GROUND_Y, WALL_DISTANCE);
    glTexCoord2f(4.0f, 0.0f); glVertex3f( GROUND_SIZE, GROUND_Y, WALL_DISTANCE);
    glTexCoord2f(4.0f, 1.0f); glVertex3f( GROUND_SIZE, WALL_HEIGHT, WALL_DISTANCE);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-GROUND_SIZE, WALL_HEIGHT, WALL_DISTANCE);
    glEnd();
    
    // East wall
    glBegin(GL_QUADS);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(WALL_DISTANCE, GROUND_Y, -GROUND_SIZE);
    glTexCoord2f(4.0f, 0.0f); glVertex3f(WALL_DISTANCE, GROUND_Y,  GROUND_SIZE);
    glTexCoord2f(4.0f, 1.0f); glVertex3f(WALL_DISTANCE, WALL_HEIGHT,  GROUND_SIZE);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(WALL_DISTANCE, WALL_HEIGHT, -GROUND_SIZE);
    glEnd();
    
    // West wall
    glBegin(GL_QUADS);
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-WALL_DISTANCE, GROUND_Y, -GROUND_SIZE);
    glTexCoord2f(4.0f, 0.0f); glVertex3f(-WALL_DISTANCE, GROUND_Y,  GROUND_SIZE);
    glTexCoord2f(4.0f, 1.0f); glVertex3f(-WALL_DISTANCE, WALL_HEIGHT,  GROUND_SIZE);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-WALL_DISTANCE, WALL_HEIGHT, -GROUND_SIZE);
    glEnd();
    
    wallTexture.UnbindTexture();  // Disable wall texture
    glDisable(GL_TEXTURE_2D);  // Disable texturing after drawing walls
}

void specialKeys(int key, int x, int y) {
    // Currently not used
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
    if (currentState != PLAYING) return;

    float speed = 0.5f;
    if (keyStates[' ']) { // Space bar for sprint
        speed = 1.0f;
    }

    Vector3 forward = camera.m_vView - camera.m_vPosition;
    forward.y = 0; // Keep movement in horizontal plane
    forward.Normalize();

    Vector3 right = forward.Cross(camera.m_vUpVector);
    right.Normalize();

    // Store the current position
    Vector3 newPosition = camera.m_vPosition;
    Vector3 movement(0, 0, 0);

    // Calculate potential movement
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

    // Calculate new position
    newPosition = newPosition + movement;

    // Check wall collisions
    const float WALL_BUFFER = 1.0f;  // Buffer distance from walls
    bool collision = false;

    // Check X-axis walls (East and West)
    if (newPosition.x >= WALL_DISTANCE - WALL_BUFFER || newPosition.x <= -WALL_DISTANCE + WALL_BUFFER) {
        newPosition.x = camera.m_vPosition.x;  // Revert X movement
        collision = true;
    }

    // Check Z-axis walls (North and South)
    if (newPosition.z >= WALL_DISTANCE - WALL_BUFFER || newPosition.z <= -WALL_DISTANCE + WALL_BUFFER) {
        newPosition.z = camera.m_vPosition.z;  // Revert Z movement
        collision = true;
    }

    // If no collision, update position
    if (!collision) {
        camera.m_vPosition = newPosition;
        camera.m_vView = camera.m_vView + movement;
    } else {
        // If there is a collision, allow sliding along the wall
        Vector3 allowedMovement = newPosition - camera.m_vPosition;
        if (!collision || abs(allowedMovement.x) > 0.01f || abs(allowedMovement.z) > 0.01f) {
            camera.m_vPosition = camera.m_vPosition + allowedMovement;
            camera.m_vView = camera.m_vView + allowedMovement;
        }
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

// ... rest of the code ... 