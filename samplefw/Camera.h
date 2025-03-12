#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../src/Frustum.h"
#include "iostream"

class Camera
{
public:
    virtual ~Camera() = default;

    virtual void update(float dt) = 0;
    virtual glm::mat4 getViewMatrix() = 0;
    virtual glm::mat4 getProjMatrix(int width, int height) = 0;

    virtual Frustum getFrustum(int width, int height)
    {
        glm::mat4 proj = getProjMatrix(width, height);
        glm::mat4 view = getViewMatrix();
        glm::mat4 clip = proj * view;

        Frustum frustum;

        // Correctly extract planes using row-major order
        frustum.planes[0] = glm::vec4(clip[0][3] + clip[0][0], clip[1][3] + clip[1][0], clip[2][3] + clip[2][0], clip[3][3] + clip[3][0]); // Left
        frustum.planes[1] = glm::vec4(clip[0][3] - clip[0][0], clip[1][3] - clip[1][0], clip[2][3] - clip[2][0], clip[3][3] - clip[3][0]); // Right
        frustum.planes[2] = glm::vec4(clip[0][3] + clip[0][1], clip[1][3] + clip[1][1], clip[2][3] + clip[2][1], clip[3][3] + clip[3][1]); // Bottom
        frustum.planes[3] = glm::vec4(clip[0][3] - clip[0][1], clip[1][3] - clip[1][1], clip[2][3] - clip[2][1], clip[3][3] - clip[3][1]); // Top
        frustum.planes[4] = glm::vec4(clip[0][2], clip[1][2], clip[2][2], clip[3][2]);  // Near (Fixed Near Plane)
        frustum.planes[5] = glm::vec4(clip[0][3] - clip[0][2], clip[1][3] - clip[1][2], clip[2][3] - clip[2][2], clip[3][3] - clip[3][2]); // Far

        // Normalize each plane to ensure correct distance calculations
        for (int i = 0; i < 6; ++i) {
            float length = glm::length(glm::vec3(frustum.planes[i]));
            if (length > 0.00001f)
                frustum.planes[i] /= length;
        }

        return frustum;
    }
};
