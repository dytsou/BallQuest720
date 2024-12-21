#ifndef CAMERA_H
#define CAMERA_H

#include "Vector3.h"
#include <GL/glut.h>

class CCamera {
public:
    Vector3 m_vPosition;	    // Camera position
    Vector3 m_vView;		    // View vector
    Vector3 m_vUpVector;	    // Up vector

    CCamera();

    void PositionCamera(float positionX, float positionY, float positionZ,
                       float viewX,     float viewY,     float viewZ,
                       float upVectorX, float upVectorY, float upVectorZ);

    void Look();
};

#endif // CAMERA_H 