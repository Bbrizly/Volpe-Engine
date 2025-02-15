#pragma once
#include "Node.h"
#include "Vertex.h"
#include "SphereVolume.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "DebugRender.h"
#include <iostream>

class DebugSphere : public Node {
private:
    void Render(const glm::mat4& proj, const glm::mat4& view, bool skipBind);
    void pushVertexData(volpe::VertexBuffer*& vBuffer,
                        volpe::VertexDeclaration*& vDecl,
                        const std::vector<Vertex>& inVerts);
    void genVertexData(); // generates the sphere geometry
    void _genVerts(int sectorCount, int stackCount);

    // Data members.
    volpe::VertexBuffer* m_vertexBuffer = nullptr;
    volpe::VertexDeclaration* m_vertexDecl = nullptr;
    volpe::Program* m_pProgram = nullptr;
    int m_numIndices = 0; // number of indices in the index buffer

    float m_radius;       // Sphere radius.
    GLubyte r = 255, g = 255, b = 255;

    // We use a sphere volume for bounding volume tests.
    SphereVolume* m_boundingVolumeSphere = nullptr;

public:
    DebugSphere(const std::string& name, float radius = 0.5f);
    virtual ~DebugSphere();

    virtual void draw(const glm::mat4& proj, const glm::mat4& view, bool skipBind = false);

    void DrawBoundingVolume(const glm::mat4& proj, const glm::mat4& view);

    void setColor(GLubyte inR, GLubyte inG, GLubyte inB) {
        r = inR; g = inG; b = inB; 
        genVertexData();}

    void setRadius(float radius) {m_radius = radius;}
};

