#include "DebugRender.h"

using namespace std;

DebugRender::DebugRender() : m_vertexBuffer(nullptr), m_vertexDecl(nullptr) {
    m_pProgram = volpe::ProgramManager::CreateProgram("data/Debug.vsh", "data/Debug.fsh");
}

DebugRender::~DebugRender() {}

DebugRender& DebugRender::Instance() {
    static DebugRender instance;
    return instance;
}

void DebugRender::DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color) {
    glm::vec3 adjustedStart = start + glm::vec3(0.0f, 0.5f, 0.0f);
    glm::vec3 adjustedEnd = end + glm::vec3(0.0f, 0.5f, 0.0f);

    vertices.push_back({adjustedStart, color});
    vertices.push_back({adjustedEnd, color});
    // vertices.push_back({start, color});
    // vertices.push_back({end, color});

    pushVertexData();
}

void DebugRender::DrawSquare(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4, const glm::vec3& color) {
    DrawLine(p1, p2, color);
    DrawLine(p2, p3, color);
    DrawLine(p3, p4, color);
    DrawLine(p4, p1, color);
    
    // pushVertexData();
}

void DebugRender::DrawCircle(const glm::vec3& center, float radius, const glm::vec3& color) {
    int segments = 16;
    for (int i = 0; i < segments; i++) {
        float theta1 = i * 2.0f * 3.14159f / segments;
        float theta2 = (i + 1) * 2.0f * 3.14159f / segments;

        glm::vec3 p1 = center + glm::vec3(radius * cos(theta1), 0, radius * sin(theta1));
        glm::vec3 p2 = center + glm::vec3(radius * cos(theta2), 0, radius * sin(theta2));

        DrawLine(p1, p2, color);
    }
    
    // pushVertexData();
}

void DebugRender::DrawSphere(const glm::vec3& center, float radius, const glm::vec3& color) {
    int segments = 16;
    for (int i = 0; i < segments; i++) {
        float theta1 = i * 2.0f * 3.14159f / segments;
        float theta2 = (i + 1) * 2.0f * 3.14159f / segments;

        glm::vec3 p1 = center + glm::vec3(radius * cos(theta1), 0, radius * sin(theta1));
        glm::vec3 p2 = center + glm::vec3(radius * cos(theta2), 0, radius * sin(theta2));

        DrawLine(p1, p2, color);
    }
    for (int i = 0; i < segments; i++) {
        float theta1 = i * 2.0f * 3.14159f / segments;
        float theta2 = (i + 1) * 2.0f * 3.14159f / segments;

        glm::vec3 p1 = center + glm::vec3(0, radius * cos(theta1), radius * sin(theta1));
        glm::vec3 p2 = center + glm::vec3(0, radius * cos(theta2), radius * sin(theta2));

        DrawLine(p1, p2, color);
    }
    
    // pushVertexData();
}

void DebugRender::pushVertexData() {
    // if (m_vertexBuffer) {
    //     volpe::BufferManager::DestroyBuffer(m_vertexBuffer);
    //     m_vertexBuffer = nullptr;
    // }
    // if (m_vertexDecl) {
    //     delete m_vertexDecl;
    //     m_vertexDecl = nullptr;
    // }
    if (vertices.empty()) return;

    m_vertexBuffer = volpe::BufferManager::CreateVertexBuffer(vertices.data(), vertices.size() * sizeof(DebugVertex));

    m_vertexDecl = new volpe::VertexDeclaration();
    m_vertexDecl->Begin();
    m_vertexDecl->AppendAttribute(volpe::AT_Position, 3, volpe::CT_Float);
    m_vertexDecl->AppendAttribute(volpe::AT_Color, 3, volpe::CT_Float);
    m_vertexDecl->SetVertexBuffer(m_vertexBuffer);
    m_vertexDecl->End();
}

void DebugRender::Render(const glm::mat4& proj, const glm::mat4& view) {
    if (vertices.empty()) return;

    // for(auto x: vertices){std::cout<<x.position.x<<", "<<x.position.y<<", "<<x.position.z<<".\n";}

    glm::mat4 world = glm::mat4(1.0f);
    glm::mat4 worldIT = glm::transpose(glm::inverse(world));

    m_pProgram->Bind();
    m_pProgram->SetUniform("projection", proj);
    m_pProgram->SetUniform("view", view);
    m_pProgram->SetUniform("world", world);
    m_pProgram->SetUniform("worldIT", worldIT);

    m_vertexDecl->Bind();
    glDrawArrays(GL_LINES, 0, vertices.size());
}

void DebugRender::Clear() {
    if(!vertices.empty())
        vertices.clear();
}
