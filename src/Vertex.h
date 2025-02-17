#pragma once
#include <GL/glew.h>

struct Vertex {
    GLfloat x, y, z;       // Position
    GLfloat nx, ny, nz;    // Normal 
    GLfloat u, v;          // UV 

    Vertex()
        : x(0), y(0), z(0),
          nx(0), ny(0), nz(0),
          u(0.0f), v(0.0f)
    {}

    Vertex(GLfloat px, GLfloat py, GLfloat pz, 
      GLfloat pnx, GLfloat pny, GLfloat pnz,
           GLfloat pu, GLfloat pv)
        : x(px), y(py), z(pz),
          nx(pnx), ny(pny), nz(pnz),
          u(pu), v(pv)
    {}
};