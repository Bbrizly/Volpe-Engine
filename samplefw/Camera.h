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
    // virtual Frustum getFrustum(int width, int height) = 0;

    
    /*
    Frustum getFrustum(int width, int height)
    {
        glm::mat4 proj = getProjMatrix(width, height);
        glm::mat4 view = getViewMatrix();
        glm::mat4 clip = proj * view;
        glm::mat4 clipT = glm::transpose(clip); // Transpose to access rows as columns
        
        Frustum frustum;
        frustum.planes[0] = clipT[3] + clipT[0]; // Left
        frustum.planes[1] = clipT[3] - clipT[0]; // Right
        frustum.planes[2] = clipT[3] + clipT[1]; // Bottom
        frustum.planes[3] = clipT[3] - clipT[1]; // Top
        frustum.planes[4] = clipT[3] + clipT[2]; // Near
        frustum.planes[5] = clipT[3] - clipT[2]; // Far

        // Normalize each plane
        for (int i = 0; i < 6; ++i) {
            float len = glm::length(glm::vec3(frustum.planes[i]));
            if (len > 0.00001f)
                frustum.planes[i] /= len;
        }
        return frustum;
    }
    */
// 
/*  
    virtual Frustum getFrustum(int width, int height)
    {
        glm::mat4 proj = getProjMatrix(width, height);
        glm::mat4 view = getViewMatrix();
        glm::mat4 clip = proj * view;
        
        Frustum frustum;
        // Using GLM’s column-major layout, each column is clip[0], clip[1], clip[2], clip[3].
        // The standard extraction is:
        frustum.planes[0] = clip[3] + clip[0]; // Left
        frustum.planes[1] = clip[3] - clip[0]; // Right
        frustum.planes[2] = clip[3] + clip[1]; // Bottom
        frustum.planes[3] = clip[3] - clip[1]; // Top
        frustum.planes[4] = clip[3] + clip[2]; // Near
        frustum.planes[5] = clip[3] - clip[2]; // Far

        // Normalize each plane:
        for (int i = 0; i < 6; ++i) {
            float len = glm::length(glm::vec3(frustum.planes[i]));
            if (len > 0.00001f)
                frustum.planes[i] /= len;
        }
        return frustum;
    }
*/

// /*

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


// */

    /*
    Frustum getFrustum(int width, int height)
    {
        glm::mat4 proj = getProjMatrix(width, height);
        glm::mat4 view = getViewMatrix();
        glm::mat4 clip = proj * view;
        
        Frustum frustum;
        
        // Left plane
        frustum.planes[0] = glm::vec4(
            clip[0][3] + clip[0][0],
            clip[1][3] + clip[1][0],
            clip[2][3] + clip[2][0],
            clip[3][3] + clip[3][0]
        );
        // Right plane
        frustum.planes[1] = glm::vec4(
            clip[0][3] - clip[0][0],
            clip[1][3] - clip[1][0],
            clip[2][3] - clip[2][0],
            clip[3][3] - clip[3][0]
        );
        // Bottom plane
        frustum.planes[2] = glm::vec4(
            clip[0][3] + clip[0][1],
            clip[1][3] + clip[1][1],
            clip[2][3] + clip[2][1],
            clip[3][3] + clip[3][1]
        );
        // Top plane
        frustum.planes[3] = glm::vec4(
            clip[0][3] - clip[0][1],
            clip[1][3] - clip[1][1],
            clip[2][3] - clip[2][1],
            clip[3][3] - clip[3][1]
        );
        // Near plane
        frustum.planes[4] = glm::vec4(
            clip[0][3] + clip[0][2],
            clip[1][3] + clip[1][2],
            clip[2][3] + clip[2][2],
            clip[3][3] + clip[3][2]
        );
        // Far plane
        frustum.planes[5] = glm::vec4(
            clip[0][3] - clip[0][2],
            clip[1][3] - clip[1][2],
            clip[2][3] - clip[2][2],
            clip[3][3] - clip[3][2]
        );
        
        // Normalize each plane (do not flip the w component)
        for (int i = 0; i < 6; i++) {
            float len = glm::length(glm::vec3(frustum.planes[i]));
            if (len > 0.00001f)
                frustum.planes[i] /= len;
        }
        for (int i = 0; i < 6; ++i) {
            std::cout << "Plane " << i << ": Normal (" 
                      << frustum.planes[i].x << ", " 
                      << frustum.planes[i].y << ", " 
                      << frustum.planes[i].z << ") | Distance: " 
                      << frustum.planes[i].w << std::endl;
        }
        
        
        return frustum;
    }
    */
};
