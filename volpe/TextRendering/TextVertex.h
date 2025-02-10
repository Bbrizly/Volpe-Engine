#pragma once
#include <GL/glew.h>

struct TextVertex {
    GLfloat x, y, z;
    GLubyte r, g, b, a;
    GLfloat u, v;
    GLfloat layer;
};