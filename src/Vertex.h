#pragma once
#include <GL/glew.h>

struct Vertex {
    GLfloat x, y, z;       // Position
    GLubyte r, g, b, a;    // Color
    GLfloat u, v;          // UV 
    GLfloat nx, ny, nz;    // Normal 

    Vertex()
        : x(0), y(0), z(0),
          r(255), g(255), b(255), a(255),
          u(0.0f), v(0.0f),
          nx(0), ny(0), nz(1)
    {}

    Vertex(GLfloat px, GLfloat py, GLfloat pz, 
           GLubyte pr, GLubyte pg, GLubyte pb, GLubyte pa, 
           GLfloat pu, GLfloat pv,
           GLfloat pnx, GLfloat pny, GLfloat pnz)
        : x(px), y(py), z(pz),
          r(pr), g(pg), b(pb), a(pa),
          u(pu), v(pv),
          nx(pnx), ny(pny), nz(pnz)
    {}
};