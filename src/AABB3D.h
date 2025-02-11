#pragma once
#include <GL/glew.h>
#include "Frustum.h"

struct AABB3D {
  glm::vec3 min;
  glm::vec3 max;

  bool IntersectsFrustum(const Frustum& frustum) const;
};
