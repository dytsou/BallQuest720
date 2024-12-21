#include "../include/Texture.h"
#include <iostream>
#include <fstream>
#include <cstring>

// Bitmap file header structure
#pragma pack(1)
typedef struct {
    unsigned short type;                 // Magic identifier
    unsigned int size;                  // File size in bytes
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int offset;                // Offset to image data in bytes
} BMPHeader;

// Bitmap information header structure
typedef struct {
    unsigned int size;               // Header size in bytes
    int width, height;                // Width and height of image
    unsigned short planes;       // Number of colour planes
    unsigned short bits;        // Bits per pixel
    unsigned int compression;        // Compression type
    unsigned int imagesize;          // Image size in bytes
    int xresolution, yresolution;     // Pixels per meter
    unsigned int ncolours;           // Number of colours
    unsigned int importantcolours;   // Important colours
} BMPInfoHeader;
#pragma pack()

CTexture::CTexture() : m_textureID(0), m_width(0), m_height(0), m_imageData(nullptr) {
}

CTexture::~CTexture() {
    if (m_imageData) {
        delete[] m_imageData;
    }
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
    }
}

int LoadBitmap(char* filename) {
    BMPHeader header;
    BMPInfoHeader infoHeader;
    GLuint texture;

    // Open the file
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Couldn't open the file " << filename << std::endl;
        return -1;
    }

    // Read the headers
    file.read(reinterpret_cast<char*>(&header), sizeof(BMPHeader));
    file.read(reinterpret_cast<char*>(&infoHeader), sizeof(BMPInfoHeader));

    // Check if it's a valid bitmap file
    if (header.type != 0x4D42) { // 'BM' in hex
        std::cerr << "Error: Not a valid bitmap file" << std::endl;
        file.close();
        return -1;
    }

    // Allocate memory for the image data
    int imageSize = infoHeader.width * infoHeader.height * 3;
    unsigned char* imageData = new unsigned char[imageSize];

    // Seek to the image data
    file.seekg(header.offset, std::ios::beg);

    // Read the image data
    file.read(reinterpret_cast<char*>(imageData), imageSize);

    // Close the file
    file.close();

    // Convert BGR to RGB
    for (int i = 0; i < imageSize; i += 3) {
        unsigned char temp = imageData[i];
        imageData[i] = imageData[i + 2];
        imageData[i + 2] = temp;
    }

    // Generate and bind texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Create the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, infoHeader.width, infoHeader.height,
                 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);

    // Free the image data
    delete[] imageData;

    return texture;
}

bool CTexture::LoadTexture(const char* filename) {
    // Delete existing texture if any
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
    }

    m_textureID = LoadBitmap(const_cast<char*>(filename));
    return m_textureID != -1;
}

void CTexture::BindTexture() {
    if (m_textureID != 0) {
        glBindTexture(GL_TEXTURE_2D, m_textureID);
    }
}

void CTexture::UnbindTexture() {
    glBindTexture(GL_TEXTURE_2D, 0);
} 