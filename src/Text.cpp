#include "../include/Text.h"

void Text::RenderText(float x, float y, const std::string& text) {
    glPushMatrix();
    glLoadIdentity();

    // Switch to 2D orthographic projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    gluOrtho2D(0, w, h, 0);

    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    // Position the text
    glRasterPos2f(x, y);

    // Render each character
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    // Restore the original projection matrix
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
} 