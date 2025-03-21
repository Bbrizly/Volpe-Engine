#ifndef FIRSTPERSONCAMERA_H
#define FIRSTPERSONCAMERA_H

#include "Camera.h"
#include "../volpe/Volpe.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class FirstPersonCamera : public Camera
{
public:
    FirstPersonCamera(volpe::App* pApp);
    ~FirstPersonCamera();

    void update(float dt) override;
    glm::mat4 getViewMatrix() override;
    glm::mat4 getProjMatrix(int width, int height) override;
    // Frustum getFrustum(int width, int height) override;

    glm::vec3 getPosition() const { return m_position; }
    glm::vec3 getDirection() const { return m_direction; }

    //move to private
    float m_fov= glm::radians(5000.0f);
    float m_near = 0.1f;
    float m_far  = 80.0f;

private:
    void _updateOrientation(const glm::vec2& mouseMovement);

    volpe::App* m_pApp;

    glm::vec3 m_position = glm::vec3(0.0f,0.0f,0.0f);
    glm::vec3 m_direction;
    glm::vec3 m_up;
    float m_yaw;
    float m_pitch;
    bool m_invertY;
    bool lockMouse = true;

    float m_normalSpeed = 20.0f;
    float m_SprintSpeed = 100.0f;

    glm::vec2 m_lastMousePos;
    float m_movementSpeed;
    float m_mouseSensitivity;
};

#endif