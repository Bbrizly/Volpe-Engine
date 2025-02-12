#pragma once

#include "Node.h"
#include "Vertex.h"
#include "../volpe/Volpe.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "iostream"
#include "AABBVolume.h"
#include "BoundingVolume.h"
#include <random>

using namespace std;
using namespace glm;


class DebugCube : public Node
{
private:

    GLubyte r=255
           ,g=255
           ,b=255;

    float cubeSize = 10.0f;

    volpe::VertexBuffer* m_vertexBuffer = nullptr;
    volpe::VertexDeclaration* m_vertexDecl = nullptr;
    volpe::Program* m_pProgram = 0;
    int m_numVertices=0;

    void Render(const glm::mat4& proj, const glm::mat4& view, bool skipBind = false);

    void pushVertexData(volpe::VertexBuffer*& vBuffer,
                             volpe::VertexDeclaration*& vDecl,
                             const vector<Vertex>& inVerts);
    void genVertexData();

public:
    DebugCube(const std::string& name);
    virtual ~DebugCube();
    
    AABBVolume getWorldAABB3D() const;
    
    std::vector<int> m_affectingLights;
    int        m_numLightsAffecting = 0;
    
    void SetProgram(volpe::Program* prog) { m_pProgram = prog; }

    void setColor(GLubyte inR, GLubyte inG, GLubyte inB) {r= inR; g = inG; b = inB; genVertexData();}

    volpe::Program* GetProgram() const { return m_pProgram; }

    // Override draw so we can actually render a cube
    virtual void draw(const glm::mat4& proj, const glm::mat4& view, bool skipBind = false);


};
