#ifndef DEBUGRENDER_H
#define DEBUGRENDER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include "../volpe/Volpe.h"
#include "Frustum.h"
#include "DebugLayer.h"
#include "../samplefw/Camera.h"
#include "../samplefw/FirstPersonCamera.h"
#include "../samplefw/OrbitCamera.h"

class DebugRender {
public:
    static DebugRender& Instance();

    void DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color = glm::vec3(1.0f), const std::string& layerName = "default");

    // Convenience functions
    void DrawSquare(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4, const glm::vec3& color = glm::vec3(1.0f), const std::string& layerName = "default");
    void DrawCircle(const glm::vec3& center, float radius, const glm::vec3& color = glm::vec3(1.0f), const std::string& layerName = "default");
    void DrawSphere(const glm::vec3& center, float radius, const glm::vec3& color, const std::string& layerName = "default");
    void DrawFrustum(const Frustum& frustum, const std::string& layerName = "default");

    // deal with a debug layer by name
    DebugLayer* GetLayer(const std::string& name);
    void CreateLayer(const std::string& name);
    void SetLayerEnabled(const std::string& name, bool enabled);
    void DrawFrustumFromCamera(Camera* cam, int screenWidth, int screenHeight, const std::string& layerName);
    // Clear all debug layers.
    void Clear();

    // Render all enabled debug layers in one draw call.
    void Render(const glm::mat4& proj, const glm::mat4& view);

private:
    DebugRender();
    ~DebugRender();

    // Helper: update (or recreate) the dynamic vertex buffer.
    void UpdateBuffer(const std::vector<float>& vertexData);

    // Aggregated debug vertex structure.
    struct DebugVertex {
        glm::vec3 position;
        glm::vec3 color;
    };

    volpe::VertexBuffer* m_vertexBuffer = nullptr;
    volpe::VertexDeclaration* m_vertexDecl = nullptr;
    volpe::Program* m_pProgram = nullptr;

    // All debug layers, keyed by name.
    std::unordered_map<std::string, DebugLayer*> m_layers;

    // Disallow copying.
    DebugRender(const DebugRender&) = delete;
    DebugRender& operator=(const DebugRender&) = delete;
};

#endif // DEBUGRENDER_H
