#pragma once
#include <GL/glew.h>

struct Vertex {
    GLfloat x, y, z;
    GLubyte r, g, b, a;
    GLfloat u, v;
    GLfloat layer;
};