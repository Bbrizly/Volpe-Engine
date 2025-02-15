#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../src/Frustum.h"

class Camera
{
public:
    virtual ~Camera() = default;

    virtual void update(float dt) = 0;
    virtual glm::mat4 getViewMatrix() = 0;
    virtual glm::mat4 getProjMatrix(int width, int height) = 0;
    // virtual Frustum getFrustum(int width, int height) = 0;
    Frustum getFrustum(int width, int height)
    {
        glm::mat4 proj = getProjMatrix(width, height);
        glm::mat4 view = getViewMatrix();
        glm::mat4 clip = proj * view;
        
        Frustum frustum;
        
        // Extract left plane
        frustum.planes[0] = glm::vec4(
            clip[0][3] + clip[0][0],
            clip[1][3] + clip[1][0],
            clip[2][3] + clip[2][0],
            clip[3][3] + clip[3][0]
        );
        // Extract right plane
        frustum.planes[1] = glm::vec4(
            clip[0][3] - clip[0][0],
            clip[1][3] - clip[1][0],
            clip[2][3] - clip[2][0],
            clip[3][3] - clip[3][0]
        );
        // Extract bottom plane
        frustum.planes[2] = glm::vec4(
            clip[0][3] + clip[0][1],
            clip[1][3] + clip[1][1],
            clip[2][3] + clip[2][1],
            clip[3][3] + clip[3][1]
        );
        // Extract top plane
        frustum.planes[3] = glm::vec4(
            clip[0][3] - clip[0][1],
            clip[1][3] - clip[1][1],
            clip[2][3] - clip[2][1],
            clip[3][3] - clip[3][1]
        );
        // Extract near plane
        frustum.planes[4] = glm::vec4(
            clip[0][3] + clip[0][2],
            clip[1][3] + clip[1][2],
            clip[2][3] + clip[2][2],
            clip[3][3] + clip[3][2]
        );
        // Extract far plane
        frustum.planes[5] = glm::vec4(
            clip[0][3] - clip[0][2],
            clip[1][3] - clip[1][2],
            clip[2][3] - clip[2][2],
            clip[3][3] - clip[3][2]
        );
        
        // // Normalize all planes
        // for (int i = 0; i < 6; ++i) {
        //     float length = glm::length(glm::vec3(frustum.planes[i]));
        //     frustum.planes[i] /= length;
        // }
        
        // return frustum;
        for(int i=0; i<6; i++){
            float len = glm::length(glm::vec3(frustum.planes[i]));
            if(fabs(len) > 0.00001f)
                frustum.planes[i] /= len;
        }
        return frustum;
    }
    
};
