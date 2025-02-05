#pragma once

#include "Node.h"
#include "Vertex.h"
#include "../volpe/Volpe.h"
#include <vector>
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

// Simple “debug” geometry
class DebugCube : public Node
{
private:
    float cubeSize = 10.0f;

    volpe::VertexBuffer* m_vertexBuffer = nullptr;
    volpe::VertexDeclaration* m_vertexDecl = nullptr;
    volpe::Program* m_pProgram = 0;
    int m_numVertices=0;

    void Render(const glm::mat4& proj, const glm::mat4& view);

    void pushVertexData(volpe::VertexBuffer*& vBuffer,
                             volpe::VertexDeclaration*& vDecl,
                             const vector<Vertex>& inVerts);
    void genVertexData();

public:
    DebugCube(const std::string& name);
    virtual ~DebugCube();

    // Override draw so we can actually render a cube
    virtual void draw(const glm::mat4& proj, const glm::mat4& view)override;//const glm::mat4& viewProj) override;


};
