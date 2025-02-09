#ifndef FRUSTUM_H
#define FRUSTUM_H

#include <glm/glm.hpp>

//left, right, bottom, top, near, far.
struct Frustum {
    glm::vec4 planes[6];
};

#endif // FRUSTUM_H
