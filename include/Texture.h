#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glut.h>
#include <string>

// Texture loading function
int LoadBitmap(char* filename);

// Texture class
class CTexture {
public:
    CTexture();
    ~CTexture();

    bool LoadTexture(const char* filename);
    void BindTexture();
    void UnbindTexture();

private:
    GLuint m_textureID;
    int m_width;
    int m_height;
    unsigned char* m_imageData;
};

#endif // TEXTURE_H 