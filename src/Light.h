#pragma once
#include <glm/glm.hpp>

class Light
{
public:
    glm::vec3 position;   // world position
    float     radius;  // distance
    glm::vec3 color;      // colorr
    float     intensity;  // Brightness

    Light(const glm::vec3& pos, const glm::vec3& col, float i, float r)
        : position(pos), color(col), intensity(i), radius(r) {}
    Light() 
        : position(0,0,0), color(glm::vec3(0)), intensity(1.0f), radius(1.0f) {}
};
