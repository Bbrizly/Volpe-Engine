#include "DebugCube.h"

using namespace std;
static const glm::vec3 cubeVertices[] = {
    {-0.5f, -0.5f,  0.5f}, {0.5f, -0.5f,  0.5f}, {0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f}, // Front face
    {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f,  0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f}  // Back face
};

static const glm::vec3 cubeNormals[] = {
    { 0.0f,  0.0f,  1.0f}, { 1.0f,  0.0f,  0.0f}, { 0.0f,  0.0f, -1.0f}, 
    {-1.0f,  0.0f,  0.0f}, { 0.0f,  1.0f,  0.0f}, { 0.0f, -1.0f,  0.0f}
};

static const glm::vec2 cubeUVs[] = {
    {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}
};

static unsigned int cubeIndices[] = {
    // front face
    0,1,2,  2,3,0,
    // right face
    1,5,6,  6,2,1,
    // back face
    5,4,7,  7,6,5,
    // left face
    4,0,3,  3,7,4,
    // top face
    3,2,6,  6,7,3,
    // bottom face
    4,5,1,  1,0,4
};

static const glm::vec3 localCorners[8] = {
    { -0.5f, -0.5f, -0.5f },
    {  0.5f, -0.5f, -0.5f },
    { -0.5f,  0.5f, -0.5f },
    {  0.5f,  0.5f, -0.5f },
    { -0.5f, -0.5f,  0.5f },
    {  0.5f, -0.5f,  0.5f },
    { -0.5f,  0.5f,  0.5f },
    {  0.5f,  0.5f,  0.5f }
};

void DebugCube::DrawBoundingVolume(const glm::mat4& proj, const glm::mat4& view)
{
    // Get the current world-space AABB
    AABBVolume box = getWorldAABB3D();
    glm::vec3 min = box.min;
    glm::vec3 max = box.max;

    // Compute the eight corners of the AABB
    glm::vec3 corners[8] = {
        glm::vec3(min.x, min.y, min.z), // 0
        glm::vec3(max.x, min.y, min.z), // 1
        glm::vec3(max.x, max.y, min.z), // 2
        glm::vec3(min.x, max.y, min.z), // 3
        glm::vec3(min.x, min.y, max.z), // 4
        glm::vec3(max.x, min.y, max.z), // 5
        glm::vec3(max.x, max.y, max.z), // 6
        glm::vec3(min.x, max.y, max.z)  // 7
    };

    // Choose a color for the bounding box (green)
    glm::vec3 color(0.0f, 1.0f, 0.0f);

    // Draw bottom face
    DebugRender::Instance().DrawLine(corners[0], corners[1], color, "BoundingVolumes");
    DebugRender::Instance().DrawLine(corners[1], corners[2], color, "BoundingVolumes");
    DebugRender::Instance().DrawLine(corners[2], corners[3], color, "BoundingVolumes");
    DebugRender::Instance().DrawLine(corners[3], corners[0], color, "BoundingVolumes");

    // Draw top face
    DebugRender::Instance().DrawLine(corners[4], corners[5], color, "BoundingVolumes");
    DebugRender::Instance().DrawLine(corners[5], corners[6], color, "BoundingVolumes");
    DebugRender::Instance().DrawLine(corners[6], corners[7], color, "BoundingVolumes");
    DebugRender::Instance().DrawLine(corners[7], corners[4], color, "BoundingVolumes");

    // Draw vertical edges
    DebugRender::Instance().DrawLine(corners[0], corners[4], color, "BoundingVolumes");
    DebugRender::Instance().DrawLine(corners[1], corners[5], color, "BoundingVolumes");
    DebugRender::Instance().DrawLine(corners[2], corners[6], color, "BoundingVolumes");
    DebugRender::Instance().DrawLine(corners[3], corners[7], color, "BoundingVolumes");
}

DebugCube::DebugCube(const std::string& name)
: Node(name)
{
    // std::cout << "CUBE INIT\n";
    
    glm::vec3 minv(+cubeSize), maxv(-cubeSize); //create cube boundingbox size
    for(int i=0; i<8; i++)
    {
        minv.x = std::min(minv.x, localCorners[i].x);
        minv.y = std::min(minv.y, localCorners[i].y);
        minv.z = std::min(minv.z, localCorners[i].z);

        maxv.x = std::max(maxv.x, localCorners[i].x);
        maxv.y = std::max(maxv.y, localCorners[i].y);
        maxv.z = std::max(maxv.z, localCorners[i].z);
    }

    AABBVolume* localBox = new AABBVolume(minv, maxv);
    m_boundingVolume = localBox;
    SetBoundingVolume(localBox);
    
    //Sphere init
    //SphereVolume* localSphere = new SphereVolume(glm::vec3(0,0,0), 0.8f);
    //SetBoundingVolume(localSphere);

    m_pProgram = volpe::ProgramManager::CreateProgram("data/Unlit3d.vsh", "data/Unlit3d.fsh");
    genVertexData();
}

DebugCube::~DebugCube(){}

void DebugCube::genVertexData() {

    std::vector<Vertex> vertices;
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 6; ++j) {
            unsigned int idx = cubeIndices[i * 6 + j];
            glm::vec3 pos = cubeVertices[idx];
            glm::vec2 uv = cubeUVs[j % 4];
            glm::vec3 norm = cubeNormals[i];
            
            vertices.emplace_back(
                pos.x, pos.y, pos.z,
                r, g, b, 255,
                uv.x, uv.y,
                norm.x, norm.y, norm.z
            );
        }
    }
    pushVertexData(m_vertexBuffer, m_vertexDecl, vertices);
}

AABBVolume DebugCube::getWorldAABB3D() const
{
    AABBVolume box;
    glm::vec3 minV(1e9f), maxV(-1e9f);

    glm::mat4 world = getWorldTransform();
    for(int i=0; i<8; i++)
    {
        glm::vec4 wPos = world * glm::vec4(localCorners[i], 1.0f);
        minV.x = std::min(minV.x, wPos.x);
        minV.y = std::min(minV.y, wPos.y);
        minV.z = std::min(minV.z, wPos.z);

        maxV.x = std::max(maxV.x, wPos.x);
        maxV.y = std::max(maxV.y, wPos.y);
        maxV.z = std::max(maxV.z, wPos.z);
    }
    box.min = minV;
    box.max = maxV;
    return box;
}

void DebugCube::pushVertexData(volpe::VertexBuffer*& vBuffer, volpe::VertexDeclaration*& vDecl, const vector<Vertex>& inVerts)
{
    if(vBuffer) {
        volpe::BufferManager::DestroyBuffer(vBuffer);
        vBuffer = nullptr;
    }
    if(vDecl) {
        delete vDecl;
        vDecl = nullptr;
    }
    if(inVerts.empty()) {
        m_numVertices=0;
        return;
    }

    vBuffer = volpe::BufferManager::CreateVertexBuffer(inVerts.data(),inVerts.size()*sizeof(Vertex));

    vDecl = new volpe::VertexDeclaration();
    vDecl->Begin();
    vDecl->AppendAttribute(volpe::AT_Position, 3, volpe::CT_Float);
    vDecl->AppendAttribute(volpe::AT_Color,    4, volpe::CT_UByte);
    vDecl->AppendAttribute(volpe::AT_TexCoord1,2, volpe::CT_Float);
    vDecl->AppendAttribute(volpe::AT_Normal, 3, volpe::CT_Float);
    // vDecl->AppendAttribute(volpe::AT_TexCoord2,1, volpe::CT_Float);
    vDecl->SetVertexBuffer(vBuffer);
    vDecl->End();

    m_numVertices = (int)inVerts.size();

    // std::cout << "DebugCube created with " << m_numVertices << " vertices.\n";
}

void DebugCube::Render(const glm::mat4& proj, const glm::mat4& view, bool skipBind)
{
    if(!m_pProgram || !m_vertexBuffer || !m_vertexDecl || m_numVertices==0) {
        
        std::cerr << "RENDER SKIPPED\n";
        return;
    }

    glm::mat4 world = getWorldTransform();

    // glm::mat4 modelViewProj = proj * view * world;
    glm::mat4 worldIT = glm::transpose(glm::inverse(world));

    if(!skipBind)
        m_pProgram->Bind();

    m_pProgram->Bind();
    m_pProgram->SetUniform("projection", proj);
    m_pProgram->SetUniform("view", view);
    m_pProgram->SetUniform("world", world);
    m_pProgram->SetUniform("worldIT", worldIT);


    m_vertexDecl->Bind();
    glDrawArrays(GL_TRIANGLES, 0, m_numVertices); //GL_TRIANGLES //GL_LINES

    // std::cout << "DebugCube Draw Call Executed.\n";
}

void DebugCube::draw(const glm::mat4& proj, const glm::mat4& view, bool skipBind)
{
    // glm::mat4 world = getWorldTransform();
    // glm::mat4 mvp   = proj * view * world;
    if(skipBind)
        Render(proj,view, skipBind);
    else
        Render(proj,view);
    // Now children draw themselves
    Node::draw(proj,view);
}
