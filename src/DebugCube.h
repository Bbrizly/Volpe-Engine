#pragma once
#include "Node.h"
#include "AABBVolume.h"
#include "../volpe/Volpe.h"
#include <glm/glm.hpp>
#include <vector>

/**
 * DebugCube that shares static geometry across all instances.
 * Uses a volpe::Material for shading.
 */
class DebugCube : public Node 
{
public:
    DebugCube(const std::string& name);
    virtual ~DebugCube();

    // Just sets a uniform color
    void setColor(GLubyte r, GLubyte g, GLubyte b);

    // Override the draw from Node
    virtual void draw(const glm::mat4& proj, const glm::mat4& view, bool skipBind = false) override;

private:
    // Our chosen color for the final shape
    glm::vec3 m_color;

    // We store a bounding volume (AABBVolume) in the constructor

    // The actual geometry is shared via static:
    static bool s_inited;
    static volpe::VertexBuffer* s_vb;
    static volpe::IndexBuffer* s_ib;
    static volpe::VertexDeclaration* s_decl;
    static int s_numIndices;

    // We also store a volpe::Material pointer in Node,
    //  so we do not need a separate pointer here. We'll just 
    //  call GetMaterial() or SetMaterial() if needed.

    // Build geometry data if not yet built
    static void initGeometry();

    // Actually draw the cube using the node's material
    void Render(const glm::mat4& proj, const glm::mat4& view);
};
