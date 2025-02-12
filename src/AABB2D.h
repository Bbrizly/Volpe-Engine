#pragma once
#include <GL/glew.h>
#include "Frustum.h"

struct AABB2D { // : public BoundingVolume{
  glm::vec2 min;
  glm::vec2 max;

  bool IntersectsFrustum(const Frustum& frustum) const;

  AABB2D() : min(glm::vec2(0.0f)), max(glm::vec2(0.0f)) {}
  
  AABB2D(glm::vec2 inMin, glm::vec2 inMax) : min(inMin), max(inMax) {}
};
