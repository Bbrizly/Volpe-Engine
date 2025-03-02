#pragma once
#include "Node.h"
#include "SphereVolume.h"
#include "../volpe/Volpe.h"
#include <glm/glm.hpp>
#include <vector>

class DebugSphere : public Node
{
public:
    DebugSphere(const std::string& name, float radius=0.5f);
    virtual ~DebugSphere();

    void setColor(GLubyte r, GLubyte g, GLubyte b);
    glm::vec3 getColor() {return m_color;}
    void setRadius(float r);
    float getRadius() {return m_radius;}

    virtual void draw(const glm::mat4& proj, const glm::mat4& view) override;

private:
    glm::vec3 m_color;
    
    float m_radius;
    SphereVolume* m_boundingVolumeSphere;

    static bool s_inited;
    static volpe::VertexBuffer* s_vb;
    static volpe::IndexBuffer*  s_ib;
    static volpe::VertexDeclaration* s_decl;
    static int s_numIndices;

    static void initGeometry(int sectorCount=20, int stackCount=20);

    void Render(const glm::mat4& proj, const glm::mat4& view);
};
