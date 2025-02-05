#pragma once
#include <glm/glm.hpp>

class Light
{
public:
    glm::vec3 position;   // world position
    glm::vec3 color;      // colorr
    float     intensity;  // Brightness

    Light(const glm::vec3& pos, const glm::vec3& col, float i)
        : position(pos), color(col), intensity(i) {}
    Light() 
        : position(0,0,0), color(1,1,1), intensity(1.f) {}
};
