#pragma once
#include "Node.h"
#include "SphereVolume.h"
#include "../volpe/Volpe.h"
#include <glm/glm.hpp>
#include <vector>

/**
 * DebugSphere that shares static geometry across all instances,
 * uses a volpe::Material, and an internal bounding sphere.
 */
class DebugSphere : public Node
{
public:
    DebugSphere(const std::string& name, float radius=0.5f);
    virtual ~DebugSphere();

    void setColor(GLubyte r, GLubyte g, GLubyte b);
    void setRadius(float r);

    virtual void draw(const glm::mat4& proj, const glm::mat4& view, bool skipBind=false) override;

private:
    // Our chosen color (uniform)
    glm::vec3 m_color;
    // Radius used for bounding volume
    float m_radius;
    // The bounding volume pointer
    SphereVolume* m_boundingVolumeSphere;

    // Shared geometry
    static bool s_inited;
    static volpe::VertexBuffer* s_vb;
    static volpe::IndexBuffer*  s_ib;
    static volpe::VertexDeclaration* s_decl;
    static int s_numIndices;

    // Build geometry if needed
    static void initGeometry(int sectorCount=20, int stackCount=20);

    // The actual draw call
    void Render(const glm::mat4& proj, const glm::mat4& view);
};
