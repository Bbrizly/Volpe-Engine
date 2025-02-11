#ifndef DEBUG_RENDER_H
#define DEBUG_RENDER_H

#include <vector>
#include <glm/glm.hpp>
#include "../volpe/Volpe.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

class DebugRender {
public:
    static DebugRender& Instance();

    void DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color = glm::vec3(1.0f));
    void DrawSquare(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4, const glm::vec3& color = glm::vec3(1.0f));
    void DrawCircle(const glm::vec3& center, float radius, const glm::vec3& color = glm::vec3(1.0f));
    void DrawSphere(const glm::vec3& center, float radius, const glm::vec3& color);

    void Render(const glm::mat4& proj, const glm::mat4& view);
    void Clear();

private:
    DebugRender();
    ~DebugRender();

    struct DebugVertex {
        glm::vec3 position;
        glm::vec3 color;
    };

    std::vector<DebugVertex> vertices;

    volpe::VertexBuffer* m_vertexBuffer = nullptr;
    volpe::VertexDeclaration* m_vertexDecl = nullptr;
    volpe::Program* m_pProgram;

    void pushVertexData();
};

#endif
