#ifndef TEXT_H
#define TEXT_H

#include <string>
#include <GL/glut.h>

class Text {
public:
    void RenderText(float x, float y, const std::string& text);
};

#endif // TEXT_H 