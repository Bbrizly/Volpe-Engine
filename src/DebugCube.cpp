#include "DebugCube.h"
#include "../volpe/Volpe.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <vector>
#include <algorithm>

// Static members
bool DebugCube::s_inited              = false;
volpe::VertexBuffer* DebugCube::s_vb  = nullptr;
volpe::IndexBuffer*  DebugCube::s_ib  = nullptr;
volpe::VertexDeclaration* DebugCube::s_decl = nullptr;
int DebugCube::s_numIndices           = 0;

////////////////////////////////////////////////////
// Constructor / Destructor
////////////////////////////////////////////////////
DebugCube::DebugCube(const std::string& name)
: Node(name), m_color(1.0f, 1.0f, 1.0f)
{
    // 1) Create the bounding volume: an AABB from -0.5..+0.5
    AABBVolume* box = new AABBVolume(glm::vec3(-0.5f), glm::vec3(0.5f));
    // SetBoundingVolume(box);

    // 2) Create a volpe::Material for this shape
    volpe::Material* mat = volpe::MaterialManager::CreateMaterial("DebugCubeMat");
    mat->SetProgram("data/Unlit3d.vsh", "data/Unlit3d.fsh");
    mat->SetDepthTest(true);
    mat->SetDepthWrite(true);
    // Store in Node's m_pMaterial
    SetMaterial(mat);

    // 3) Build static geometry once
    initGeometry();
}

DebugCube::~DebugCube()
{
    // Node destructor won't automatically do it, so let's do it here
    if(GetMaterial()) 
    {
        volpe::MaterialManager::DestroyMaterial(GetMaterial());
        SetMaterial(nullptr);
    }
}

////////////////////////////////////////////////////
// Public
////////////////////////////////////////////////////
void DebugCube::setColor(GLubyte r, GLubyte g, GLubyte b)
{
    m_color = glm::vec3(r/255.f, g/255.f, b/255.f);
}

void DebugCube::draw(const glm::mat4& proj, const glm::mat4& view, bool skipBind)
{
    // 1) Render the actual cube
    Render(proj, view);

    // 2) Draw bounding volume lines (if present)
    if(GetBoundingVolume()) {
        GetBoundingVolume()->DrawMe(proj, view);
    }

    // 3) Let Node draw any children
    Node::draw(proj, view, skipBind);
}

////////////////////////////////////////////////////
// Private: The main geometry draw
////////////////////////////////////////////////////
void DebugCube::Render(const glm::mat4& proj, const glm::mat4& view)
{
    if(!s_inited || s_numIndices <= 0)
        return;
    volpe::Material* mat = GetMaterial();
    if(!mat) 
        return;

    // Build transform
    glm::mat4 world = getWorldTransform();
    glm::mat4 worldIT = glm::transpose(glm::inverse(world));

    // Set uniforms
    mat->SetUniform("projection", proj);
    mat->SetUniform("view",       view);
    mat->SetUniform("world",      world);
    mat->SetUniform("worldIT",    worldIT);

    // If your Unlit3d.fsh references "u_color"
    mat->SetUniform("u_color", m_color);

    // apply (bind program, set states)
    mat->Apply();

    // bind the geometry
    s_decl->Bind();
    // draw
    glDrawElements(GL_TRIANGLES, s_numIndices, GL_UNSIGNED_SHORT, 0);
}

////////////////////////////////////////////////////
// Private (static): Create geometry if not built
////////////////////////////////////////////////////
void DebugCube::initGeometry()
{
    if(s_inited)
        return;

    // We'll define a typical unit-cube index buffer (36 indices),
    // plus vertices for each corner. We can do 24-vertex style 
    // or we can do 8 corners and let each face share them with 
    // correct normals, etc. For simplicity, we do a 36 index approach.

    // We'll define 8 corners:
    static const glm::vec3 corners[8] = {
        {-0.5f,-0.5f,-0.5f}, {+0.5f,-0.5f,-0.5f}, 
        {-0.5f,+0.5f,-0.5f}, {+0.5f,+0.5f,-0.5f}, 
        {-0.5f,-0.5f,+0.5f}, {+0.5f,-0.5f,+0.5f}, 
        {-0.5f,+0.5f,+0.5f}, {+0.5f,+0.5f,+0.5f}
    };
    // We'll define these 36 indices (6 faces * 2 triangles * 3 indices):
    static const unsigned short cubeIndices[36] = {
        // back face (z=-0.5)
        0,2,1,   1,2,3,
        // front face (z=+0.5)
        4,5,6,   5,7,6,
        // left face (x=-0.5)
        0,4,2,   2,4,6,
        // right face (x=+0.5)
        1,3,5,   3,7,5,
        // top face (y=+0.5)
        2,6,3,   3,6,7,
        // bottom face (y=-0.5)
        0,1,4,   1,5,4
    };

    // We'll store for each index a Vertex that has 
    // position + normal + uv. We'll compute normal by cross product face by face or 
    // do a naive approach. Let's do face-based approach: each 6 indices is a face.

    struct Vtx {
        float x,y,z;
        float nx, ny, nz;
        float u, v;
    };
    std::vector<Vtx> finalVerts;
    finalVerts.reserve(36); // 12 triangles = 36 indices => 36 Vtx
    // We'll read them in sets of 6 => 2 triangles => 1 face normal
    for(int f=0; f<6; f++)
    {
        // each face has 6 indices 
        int faceStart = f*6;
        // compute face normal via the first triangle cross 
        // i.e. (0,2,1) or something
        unsigned short i0 = cubeIndices[faceStart + 0];
        unsigned short i1 = cubeIndices[faceStart + 1];
        unsigned short i2 = cubeIndices[faceStart + 2];

        glm::vec3 p0 = corners[i0];
        glm::vec3 p1 = corners[i1];
        glm::vec3 p2 = corners[i2];
        glm::vec3 faceNormal = glm::normalize(glm::cross(p1-p0, p2-p0));

        // fill these 6 indices with that same normal
        for(int i=0; i<6; i++)
        {
            unsigned short idx = cubeIndices[faceStart + i];
            glm::vec3 pos = corners[idx];
            Vtx v;
            v.x = pos.x; 
            v.y = pos.y;
            v.z = pos.z;
            v.nx = faceNormal.x;
            v.ny = faceNormal.y;
            v.nz = faceNormal.z;
            // Just generate uv trivially
            v.u = (faceNormal.x != 0.f) ? (pos.y + 0.5f) : (pos.x + 0.5f); 
            v.v = (faceNormal.z != 0.f) ? (pos.y + 0.5f) : (pos.z + 0.5f);
            finalVerts.push_back(v);
        }
    }
    // finalVerts now has 36. We'll do draw arrays, or build an index buffer that 
    // references them. But let's do an index buffer with 36 indices that 
    // just goes 0..35 if we want, but we can also just do glDrawArrays. 
    // We'll do an index approach:

    s_numIndices = (int)finalVerts.size(); // 36
    // We'll build a trivial index [0..35]
    std::vector<unsigned short> finalIdx;
    finalIdx.reserve(s_numIndices);
    for(int i=0; i<s_numIndices; i++)
        finalIdx.push_back((unsigned short)i);

    // Create the GPU buffers
    s_vb = volpe::BufferManager::CreateVertexBuffer(finalVerts.data(), (unsigned int) (finalVerts.size()*sizeof(Vtx)));
    s_ib = volpe::BufferManager::CreateIndexBuffer(finalIdx.data(), (unsigned int)finalIdx.size());

    s_decl = new volpe::VertexDeclaration();
    s_decl->Begin();
      s_decl->SetVertexBuffer(s_vb);
      s_decl->SetIndexBuffer(s_ib);
      s_decl->AppendAttribute(volpe::AT_Position,   3, volpe::CT_Float);
      s_decl->AppendAttribute(volpe::AT_Normal,     3, volpe::CT_Float);
      s_decl->AppendAttribute(volpe::AT_TexCoord1,  2, volpe::CT_Float);
    s_decl->End();

    s_inited = true;
}
