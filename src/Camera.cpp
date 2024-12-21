#include "../include/Camera.h"
#include <cmath>

CCamera::CCamera() {
    m_vPosition = Vector3(0, 0, 0);
    m_vView = Vector3(0, 0, 1);
    m_vUpVector = Vector3(0, 1, 0);
}

void CCamera::PositionCamera(float positionX, float positionY, float positionZ,
                           float viewX, float viewY, float viewZ,
                           float upVectorX, float upVectorY, float upVectorZ) {
    m_vPosition = Vector3(positionX, positionY, positionZ);
    m_vView = Vector3(viewX, viewY, viewZ);
    m_vUpVector = Vector3(upVectorX, upVectorY, upVectorZ);
}

void CCamera::Look() {
    gluLookAt(m_vPosition.x, m_vPosition.y, m_vPosition.z,
              m_vView.x, m_vView.y, m_vView.z,
              m_vUpVector.x, m_vUpVector.y, m_vUpVector.z);
} 