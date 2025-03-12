#pragma once
#include <glm/glm.hpp>

struct Boid
{
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;

    // Boid parameters
    float maxSpeed;       // how fast boid can go
    float maxForce;       // how strong steering can be
    float neighborRadius; // how far to look for neighbors
    float separationDist; // min distance for separation

    float rotation;       
    float size;           
    glm::vec4 color;  

    float age;      
    float lifetime;

    Boid()
        : position(0.0f),
          velocity(0.0f),
          acceleration(0.0f),
          maxSpeed(4.0f),
          maxForce(0.5f),
          neighborRadius(5.0f),
          separationDist(1.0f),
          rotation(0.0f),
          size(1.0f),
          color(1.0f),
          age(0.0f),
          lifetime(999999.0f)
    {}
};
