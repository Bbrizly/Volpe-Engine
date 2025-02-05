#pragma once

#include "../volpe/Volpe.h"

class Sphere 
{
    public:
        Sphere(float radius);
        ~Sphere();

        void SetPosition(const glm::vec3& pos) { m_pos = pos; }
        void SetRadius(float radius) { m_radius = radius; m_scale = glm::vec3(radius / BASE_RADIUS, radius / BASE_RADIUS, radius / BASE_RADIUS); }
        void SetColor(const glm::vec3& color) { m_color = color; }

        void Render(const glm::mat4& worldMatrix, const glm::mat4& viewMatrix, const glm::mat4& projMatrix);

    private:

        void _genVerts(int sectorCount, int stackCount);

        static volpe::VertexBuffer* s_pVB;
        static volpe::IndexBuffer* s_pIB;
        static volpe::VertexDeclaration* s_pDecl;
        static int s_numIndices;

        float m_radius;
        glm::vec3 m_pos;
        glm::vec3 m_scale;
        glm::vec3 m_color = glm::vec3(1.0f,1.0f,1.0f);
        volpe::Material* m_pMat;

        const float BASE_RADIUS = 0.5f;

};