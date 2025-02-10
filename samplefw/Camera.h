#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
    virtual ~Camera() = default;

    virtual void update(float dt) = 0;
    virtual glm::mat4 getViewMatrix() = 0;
    virtual glm::mat4 getProjMatrix(int width, int height) = 0;
    // virtual glm::mat4 getFrustum(int width, int height) = 0;
    
};
