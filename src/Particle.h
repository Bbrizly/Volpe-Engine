#pragma once
#include <GL/glew.h>

struct Particle {
  glm::vec3 position;
  glm::vec3 velocity;       
  glm::vec3 acceleration;   
  float     lifetime;       // total time to live (inseconds)
  float     age;            // how long it has been alive
  float     size;           // current size
  float     initialSize;    // size at birth ;; for over-time effects
  float     rotation;       // billboard rotation in degrees
  float     rotationSpeed;  
  glm::vec4 color;          // current RGBA (ADD GRADIENT FUNCTIONALITY TO EMITTER)
  glm::vec4 initialColor;   // color at birth ;; for over-time effects
  float textureIndex;  // which layer in the array
  glm::vec2 stretch;

  Particle()
      : position(0), velocity(0), acceleration(0),
        lifetime(4.0f), age(0.0f),
        size(1.0f), initialSize(1.0f),
        rotation(0.0f), rotationSpeed(0.0f),
        color(1.0f), initialColor(1.0f),
        stretch(1.0f, 1.0f) {}
};