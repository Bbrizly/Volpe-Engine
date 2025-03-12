#pragma once
#include "Node.h"
#include "../samplefw/BoundingVolumes/AABBVolume.h"
#include "../volpe/Volpe.h"
#include <glm/glm.hpp>
#include <vector>

class DebugCube : public Node 
{
public:
    DebugCube(const std::string& name);
    virtual ~DebugCube();

    glm::vec3 getColor() {return m_color;}
    void setColor(GLubyte r, GLubyte g, GLubyte b);

    virtual void draw(const glm::mat4& proj, const glm::mat4& view) override;

private:
    glm::vec3 m_color;

    static bool s_inited;
    static volpe::VertexBuffer* s_vb;
    static volpe::IndexBuffer* s_ib;
    static volpe::VertexDeclaration* s_decl;
    static int s_numIndices;

    static void initGeometry();

    void Render(const glm::mat4& proj, const glm::mat4& view);
};
