#pragma once

#include "../volpe/Volpe.h"

class Grid2D
{
    public:
        Grid2D(int linesPerHalfSpace);
        ~Grid2D();

        void update(float dt);
        void render(const glm::mat4& mView, const glm::mat4& mProj);
        void showAxes() { m_showAxes = true; }
        void hideAxes() { m_showAxes = false; }

    private:

        void _createGrid(int linesPerHalfSpace);
        void _createAxes();

        volpe::VertexBuffer* m_pVB = 0;
        volpe::VertexDeclaration* m_pDecl = 0;
        volpe::Program* m_pProgram = 0;
        volpe::Color4 m_color;

        volpe::VertexBuffer *m_pAxesVB = 0;
        volpe::VertexDeclaration *m_pAxesDecl = 0;

        int m_numVerts = 0;
        bool m_showAxes = true;
};