#pragma once

#include "Camera.h"
#include "../volpe/Volpe.h"

class OrbitCamera : public Camera
{
public:
    OrbitCamera(volpe::App* pApp);
    virtual ~OrbitCamera();

    void update(float dt) override;
    glm::mat4 getViewMatrix() override;
    glm::mat4 getProjMatrix(int width, int height) override;
    // Frustum getFrustum(int width, int height) override;

    glm::vec3 getViewDirection() const;
    glm::vec3 getViewPosition() const;

    void focusOn(const glm::vec3& min, const glm::vec3& max);
    
    float m_fov                 = glm::radians(45.0f);
    float m_near                = 0.1f;
    float m_far                 = 1000.0f;
    
private:
    void _rotate(const glm::vec2& mouseMovement);
    glm::vec3 _getCameraUp();
    glm::vec3 _getCameraSide();
    void _pan(const glm::vec2& mouseMovement);
    float _calculateRequiredDistance();

    float m_rotX                = 0.0f;
    float m_rotY                = 0.0f;
    float m_distance            = 100.0f;
    glm::vec3 m_offset          = glm::vec3(0.0f,0.0f,0.0f);
    glm::vec3 m_position        = glm::vec3(0.0f,0.0f,0.0f);
    glm::vec3 m_target          = glm::vec3(0.0f,0.0f,0.0f);
    glm::vec3 m_focusMin        = glm::vec3(0.0f,0.0f,0.0f);
    glm::vec3 m_focusMax        = glm::vec3(0.0f,0.0f,0.0f);
    glm::vec2 m_lastMousePos    = glm::vec2(0.0f,0.0f);
    bool lockMouse = true;
    volpe::App* m_pApp           = nullptr;
};