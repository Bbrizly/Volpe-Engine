#include "DebugRender.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

DebugRender& DebugRender::Instance() {
    static DebugRender instance;
    return instance;
}

DebugRender::DebugRender() {
    // Create your debug shader program (adjust paths as needed).
    m_pProgram = volpe::ProgramManager::CreateProgram("data/Debug.vsh", "data/Debug.fsh");
    // Ensure a default layer exists.
    CreateLayer("default");
}

DebugRender::~DebugRender() {
    for (auto& pair : m_layers) {
        delete pair.second;
    }
    m_layers.clear();
    if (m_vertexBuffer) {
        volpe::BufferManager::DestroyBuffer(m_vertexBuffer);
        m_vertexBuffer = nullptr;
    }
    if (m_vertexDecl) {
        delete m_vertexDecl;
        m_vertexDecl = nullptr;
    }
}

void DebugRender::CreateLayer(const std::string& name) {
    if (m_layers.find(name) == m_layers.end()) {
        m_layers[name] = new DebugLayer(name);
    }
}

DebugLayer* DebugRender::GetLayer(const std::string& name) {
    auto it = m_layers.find(name);
    if (it == m_layers.end()) {
        CreateLayer(name);
        return m_layers[name];
    }
    return it->second;
}

void DebugRender::SetLayerEnabled(const std::string& name, bool enabled) {
    DebugLayer* layer = GetLayer(name);
    if (layer) {
        layer->SetEnabled(enabled);
    }
}

void DebugRender::DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color, const std::string& layerName) {
    DebugLayer* layer = GetLayer(layerName);
    if (layer && layer->IsEnabled()) {
        layer->AddLine(start, end, color);
    }
}

void DebugRender::DrawSquare(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4, const glm::vec3& color, const std::string& layerName) {
    DrawLine(p1, p2, color, layerName);
    DrawLine(p2, p3, color, layerName);
    DrawLine(p3, p4, color, layerName);
    DrawLine(p4, p1, color, layerName);
}

void DebugRender::DrawCircle(const glm::vec3& center, float radius, const glm::vec3& color, const std::string& layerName) {
    int segments = 16;
    float angleStep = 2.0f * 3.14159f / segments;
    for (int i = 0; i < segments; ++i) {
        float theta1 = i * angleStep;
        float theta2 = (i + 1) * angleStep;
        glm::vec3 p1 = center + glm::vec3(radius * cos(theta1), 0, radius * sin(theta1));
        glm::vec3 p2 = center + glm::vec3(radius * cos(theta2), 0, radius * sin(theta2));
        DrawLine(p1, p2, color, layerName);
    }
}

void DebugRender::DrawSphere(const glm::vec3& center, float radius, const glm::vec3& color, const std::string& layerName) {
    // Draw two circles (XZ and YZ planes) as an approximation.
    int segments = 16;
    float angleStep = 2.0f * 3.14159f / segments;
    // XZ plane
    for (int i = 0; i < segments; ++i) {
        float theta1 = i * angleStep;
        float theta2 = (i + 1) * angleStep;
        glm::vec3 p1 = center + glm::vec3(radius * cos(theta1), 0, radius * sin(theta1));
        glm::vec3 p2 = center + glm::vec3(radius * cos(theta2), 0, radius * sin(theta2));
        DrawLine(p1, p2, color, layerName);
    }
    // YZ plane
    for (int i = 0; i < segments; ++i) {
        float theta1 = i * angleStep;
        float theta2 = (i + 1) * angleStep;
        glm::vec3 p1 = center + glm::vec3(0, radius * cos(theta1), radius * sin(theta1));
        glm::vec3 p2 = center + glm::vec3(0, radius * cos(theta2), radius * sin(theta2));
        DrawLine(p1, p2, color, layerName);
    }
}

void DebugRender::DrawFrustum(const Frustum& frustum, const std::string& layerName) {
    // Assuming planes are ordered: left, right, bottom, top, near, far.
    // Compute the 8 corners by intersecting planes.
    auto IntersectPlanes = [&](const glm::vec4& p1, const glm::vec4& p2, const glm::vec4& p3) -> glm::vec3 {
        glm::vec3 n1(p1.x, p1.y, p1.z);
        glm::vec3 n2(p2.x, p2.y, p2.z);
        glm::vec3 n3(p3.x, p3.y, p3.z);
        float d1 = p1.w, d2 = p2.w, d3 = p3.w;
        glm::vec3 numerator = glm::cross(n2, n3) * (-d1) + glm::cross(n3, n1) * (-d2) + glm::cross(n1, n2) * (-d3);
        float denominator = glm::dot(n1, glm::cross(n2, n3));
        return numerator / denominator;
    };

    glm::vec3 ntl = IntersectPlanes(frustum.planes[0], frustum.planes[3], frustum.planes[4]);
    glm::vec3 ntr = IntersectPlanes(frustum.planes[1], frustum.planes[3], frustum.planes[4]);
    glm::vec3 nbl = IntersectPlanes(frustum.planes[0], frustum.planes[2], frustum.planes[4]);
    glm::vec3 nbr = IntersectPlanes(frustum.planes[1], frustum.planes[2], frustum.planes[4]);
    glm::vec3 ftl = IntersectPlanes(frustum.planes[0], frustum.planes[3], frustum.planes[5]);
    glm::vec3 ftr = IntersectPlanes(frustum.planes[1], frustum.planes[3], frustum.planes[5]);
    glm::vec3 fbl = IntersectPlanes(frustum.planes[0], frustum.planes[2], frustum.planes[5]);
    glm::vec3 fbr = IntersectPlanes(frustum.planes[1], frustum.planes[2], frustum.planes[5]);
    glm::vec3 color(1.0f, 1.0f, 0.0f); // Yellow

    // Near plane edges.
    DrawLine(ntl, ntr, color, layerName);
    DrawLine(ntr, nbr, color, layerName);
    DrawLine(nbr, nbl, color, layerName);
    DrawLine(nbl, ntl, color, layerName);
    // Far plane edges.
    DrawLine(ftl, ftr, color, layerName);
    DrawLine(ftr, fbr, color, layerName);
    DrawLine(fbr, fbl, color, layerName);
    DrawLine(fbl, ftl, color, layerName);
    // Connect near and far.
    DrawLine(ntl, ftl, color, layerName);
    DrawLine(ntr, ftr, color, layerName);
    DrawLine(nbl, fbl, color, layerName);
    DrawLine(nbr, fbr, color, layerName);
}

void DebugRender::Clear() {
    // Clear all layers.
    for (auto& pair : m_layers) {
        pair.second->Clear();
    }
}

void DebugRender::UpdateBuffer(const std::vector<float>& vertexData) {
    size_t dataSize = vertexData.size() * sizeof(float);
    if (m_vertexBuffer) {
        volpe::BufferManager::DestroyBuffer(m_vertexBuffer);
        m_vertexBuffer = nullptr;
    }
    if (dataSize > 0) {
        m_vertexBuffer = volpe::BufferManager::CreateVertexBuffer(vertexData.data(), dataSize);
        if (m_vertexDecl) {
            delete m_vertexDecl;
            m_vertexDecl = nullptr;
        }
        m_vertexDecl = new volpe::VertexDeclaration();
        m_vertexDecl->Begin();
        m_vertexDecl->AppendAttribute(volpe::AT_Position, 3, volpe::CT_Float);
        m_vertexDecl->AppendAttribute(volpe::AT_Color, 3, volpe::CT_Float);
        m_vertexDecl->SetVertexBuffer(m_vertexBuffer);
        m_vertexDecl->End();
    }
}

void DebugRender::Render(const glm::mat4& proj, const glm::mat4& view) {
    std::vector<float> vertexData;
    // For each enabled layer, transform and accumulate its debug lines.
    for (const auto& pair : m_layers) {
        DebugLayer* layer = pair.second;
        if (!layer->IsEnabled())
            continue;
        const auto& lines = layer->GetLines();
        glm::mat4 transform = layer->GetTransform();
        for (const DebugLine& line : lines) {
            glm::vec4 start = transform * glm::vec4(line.start, 1.0f);
            glm::vec4 end   = transform * glm::vec4(line.end, 1.0f);
            // Start vertex.
            vertexData.push_back(start.x);
            vertexData.push_back(start.y);
            vertexData.push_back(start.z);
            vertexData.push_back(line.color.r);
            vertexData.push_back(line.color.g);
            vertexData.push_back(line.color.b);
            // End vertex.
            vertexData.push_back(end.x);
            vertexData.push_back(end.y);
            vertexData.push_back(end.z);
            vertexData.push_back(line.color.r);
            vertexData.push_back(line.color.g);
            vertexData.push_back(line.color.b);
        }
    }
    UpdateBuffer(vertexData);
    if (!vertexData.empty() && m_pProgram && m_vertexDecl) {
        m_pProgram->Bind();
        m_pProgram->SetUniform("projection", proj);
        m_pProgram->SetUniform("view", view);
        glm::mat4 world = glm::mat4(1.0f);
        m_pProgram->SetUniform("world", world);
        glm::mat4 worldIT = glm::transpose(glm::inverse(world));
        m_pProgram->SetUniform("worldIT", worldIT);
        m_vertexDecl->Bind();
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertexData.size() / 6));
    }
}
