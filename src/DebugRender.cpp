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

glm::vec3 DebugRender::IntersectPlanes(const glm::vec4& p1, const glm::vec4& p2, const glm::vec4& p3) const {
    glm::vec3 n1(p1), n2(p2), n3(p3);
    float d1 = p1.w, d2 = p2.w, d3 = p3.w;
    glm::vec3 cross23 = glm::cross(n2, n3);
    float denom = glm::dot(n1, cross23);
    if (fabs(denom) < 1e-6f) // Avoid division by zero
        return glm::vec3(0.0f);
    glm::vec3 result = (-d1 * cross23 - d2 * glm::cross(n3, n1) - d3 * glm::cross(n1, n2)) / denom;
    return result;
}

void DebugRender::DrawFrustum(const Frustum& frustum) {
    // Assuming the frustum planes are stored in the following order:
    // 0: Left, 1: Right, 2: Bottom, 3: Top, 4: Near, 5: Far

    // Compute near-plane corners:
    glm::vec3 nbl = IntersectPlanes(frustum.planes[0], frustum.planes[2], frustum.planes[4]); // left, bottom, near
    glm::vec3 nbr = IntersectPlanes(frustum.planes[1], frustum.planes[2], frustum.planes[4]); // right, bottom, near
    glm::vec3 ntr = IntersectPlanes(frustum.planes[1], frustum.planes[3], frustum.planes[4]); // right, top, near
    glm::vec3 ntl = IntersectPlanes(frustum.planes[0], frustum.planes[3], frustum.planes[4]); // left, top, near

    // Compute far-plane corners:
    glm::vec3 fbl = IntersectPlanes(frustum.planes[0], frustum.planes[2], frustum.planes[5]); // left, bottom, far
    glm::vec3 fbr = IntersectPlanes(frustum.planes[1], frustum.planes[2], frustum.planes[5]); // right, bottom, far
    glm::vec3 ftr = IntersectPlanes(frustum.planes[1], frustum.planes[3], frustum.planes[5]); // right, top, far
    glm::vec3 ftl = IntersectPlanes(frustum.planes[0], frustum.planes[3], frustum.planes[5]); // left, top, far

    // Draw near plane edges in red.
    DebugRender::Instance().DrawLine(nbl, nbr, glm::vec3(1,0,0));
    DebugRender::Instance().DrawLine(nbr, ntr, glm::vec3(1,0,0));
    DebugRender::Instance().DrawLine(ntr, ntl, glm::vec3(1,0,0));
    DebugRender::Instance().DrawLine(ntl, nbl, glm::vec3(1,0,0));

    // Draw far plane edges in green.
    DebugRender::Instance().DrawLine(fbl, fbr, glm::vec3(0,1,0));
    DebugRender::Instance().DrawLine(fbr, ftr, glm::vec3(0,1,0));
    DebugRender::Instance().DrawLine(ftr, ftl, glm::vec3(0,1,0));
    DebugRender::Instance().DrawLine(ftl, fbl, glm::vec3(0,1,0));

    // Draw the vertical edges connecting near and far planes in blue.
    DebugRender::Instance().DrawLine(nbl, fbl, glm::vec3(0,0,1));
    DebugRender::Instance().DrawLine(nbr, fbr, glm::vec3(0,0,1));
    DebugRender::Instance().DrawLine(ntr, ftr, glm::vec3(0,0,1));
    DebugRender::Instance().DrawLine(ntl, ftl, glm::vec3(0,0,1));
}

/*
void DebugRender::DrawFrustum(const glm::mat4& proj, const glm::mat4& view) {
    // Invert the view-projection matrix
    glm::mat4 invVP = glm::inverse(proj * view);
    
    // Define the 8 corners in normalized device coordinates (NDC)
    glm::vec3 ndcCorners[8] = {
        glm::vec3(-1.0f, -1.0f, -1.0f), // near-bottom-left
        glm::vec3( 1.0f, -1.0f, -1.0f), // near-bottom-right
        glm::vec3( 1.0f,  1.0f, -1.0f), // near-top-right
        glm::vec3(-1.0f,  1.0f, -1.0f), // near-top-left
        glm::vec3(-1.0f, -1.0f,  1.0f), // far-bottom-left
        glm::vec3( 1.0f, -1.0f,  1.0f), // far-bottom-right
        glm::vec3( 1.0f,  1.0f,  1.0f), // far-top-right
        glm::vec3(-1.0f,  1.0f,  1.0f)  // far-top-left
    };

    // Transform NDC corners into world space
    glm::vec3 worldCorners[8];
    for (int i = 0; i < 8; i++) {
        glm::vec4 corner = invVP * glm::vec4(ndcCorners[i], 1.0f);
        worldCorners[i] = glm::vec3(corner) / corner.w;
    }
    
    // Draw bottom face edges (using red color)
    DebugRender::Instance().DrawLine(worldCorners[0], worldCorners[1], glm::vec3(1, 0, 0));
    DebugRender::Instance().DrawLine(worldCorners[1], worldCorners[2], glm::vec3(1, 0, 0));
    DebugRender::Instance().DrawLine(worldCorners[2], worldCorners[3], glm::vec3(1, 0, 0));
    DebugRender::Instance().DrawLine(worldCorners[3], worldCorners[0], glm::vec3(1, 0, 0));
    
    // Draw top face edges (using green color)
    DebugRender::Instance().DrawLine(worldCorners[4], worldCorners[5], glm::vec3(0, 1, 0));
    DebugRender::Instance().DrawLine(worldCorners[5], worldCorners[6], glm::vec3(0, 1, 0));
    DebugRender::Instance().DrawLine(worldCorners[6], worldCorners[7], glm::vec3(0, 1, 0));
    DebugRender::Instance().DrawLine(worldCorners[7], worldCorners[4], glm::vec3(0, 1, 0));
    
    // Draw vertical edges (using blue color)
    DebugRender::Instance().DrawLine(worldCorners[0], worldCorners[4], glm::vec3(0, 0, 1));
    DebugRender::Instance().DrawLine(worldCorners[1], worldCorners[5], glm::vec3(0, 0, 1));
    DebugRender::Instance().DrawLine(worldCorners[2], worldCorners[6], glm::vec3(0, 0, 1));
    DebugRender::Instance().DrawLine(worldCorners[3], worldCorners[7], glm::vec3(0, 0, 1));
}
*/
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

    

    // if(m_vertexBuffer)
    // {
    //     volpe::BufferManager::UpdateVertexBuffer(m_vertexBuffer,vertices.data(),vertices.size() * sizeof(DebugVertex));
    //     return;
    // }

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
    
    pushVertexData();

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
