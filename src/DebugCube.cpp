#include "DebugCube.h"
#include "../volpe/Volpe.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <vector>
#include <algorithm>
#include "Vertex.h"

// Static members
bool DebugCube::s_inited              = false;
volpe::VertexBuffer* DebugCube::s_vb  = nullptr;
volpe::IndexBuffer*  DebugCube::s_ib  = nullptr;
volpe::VertexDeclaration* DebugCube::s_decl = nullptr;
int DebugCube::s_numIndices           = 0;

DebugCube::DebugCube(const std::string& name)
: Node(name), m_color(1.0f, 1.0f, 1.0f)
{
    AABBVolume* box = new AABBVolume(glm::vec3(-0.5f), glm::vec3(0.5f));
    SetBoundingVolume(box);

    volpe::Material* mat = volpe::MaterialManager::CreateMaterial("DebugCubeMat");
    mat->SetProgram("data/Unlit3d.vsh", "data/Unlit3d.fsh");
    mat->SetDepthTest(true);
    mat->SetDepthWrite(true);
    
    SetMaterial(mat);

    initGeometry();
}

DebugCube::~DebugCube()
{
    if(GetMaterial()) 
    {
        volpe::MaterialManager::DestroyMaterial(GetMaterial());
        SetMaterial(nullptr);
    }
}

void DebugCube::setColor(GLubyte r, GLubyte g, GLubyte b)
{
    m_color = glm::vec3(r/255.f, g/255.f, b/255.f);
}

void DebugCube::draw(const glm::mat4& proj, const glm::mat4& view)
{
    Render(proj, view);
    
    Node::draw(proj, view);
}

void DebugCube::Render(const glm::mat4& proj, const glm::mat4& view)
{
    if(!s_inited || s_numIndices <= 0)
        return;
    volpe::Material* mat = GetMaterial();
    if(!mat) 
        return;

    glm::mat4 world = getWorldTransform();
    glm::mat4 worldIT = glm::transpose(glm::inverse(world));

    mat->SetUniform("projection", proj);
    mat->SetUniform("view",       view);
    mat->SetUniform("world",      world);
    mat->SetUniform("worldIT",    worldIT);

    mat->SetUniform("u_color", m_color);

    mat->Apply();

    s_decl->Bind();

    glDrawElements(GL_TRIANGLES, s_numIndices, GL_UNSIGNED_SHORT, 0);
}

/*
void DebugCube::initGeometry()
{
    if (s_inited) 
        return;

    // 8 corners of the cube
    static const glm::vec3 corners[8] = {
        {-0.5f, -0.5f, -0.5f}, 
        {+0.5f, -0.5f, -0.5f},
        {-0.5f, +0.5f, -0.5f}, 
        {+0.5f, +0.5f, -0.5f},
        {-0.5f, -0.5f, +0.5f}, 
        {+0.5f, -0.5f, +0.5f},
        {-0.5f, +0.5f, +0.5f}, 
        {+0.5f, +0.5f, +0.5f}
    };

    // Index data for 12 triangles (6 faces × 2):
    static const unsigned short cubeIndices[36] = {
        0,2,1,  1,2,3,  // back
        4,5,6,  5,7,6,  // front
        0,4,2,  2,4,6,  // left
        1,3,5,  3,7,5,  // right
        2,6,3,  3,6,7,  // top
        0,1,4,  1,5,4   // bottom
    };

    std::vector<Vertex> finalVerts;
    finalVerts.reserve(36);

    // Build each face from 6 indices
    for(int face = 0; face < 6; face++)
    {
        int faceStart = face * 6;
        unsigned short i0 = cubeIndices[faceStart + 0];
        unsigned short i1 = cubeIndices[faceStart + 1];
        unsigned short i2 = cubeIndices[faceStart + 2];

        glm::vec3 p0 = corners[i0];
        glm::vec3 p1 = corners[i1];
        glm::vec3 p2 = corners[i2];

        // Cross in CCW order, so the normal points outward.
        glm::vec3 faceNormal = glm::normalize(glm::cross(p1 - p0, p2 - p0));
        std::cout << "faceNormal: (" << faceNormal.x << ", " << faceNormal.y << ", " << faceNormal.z << ")\n";


        // Fill 6 vertices for the 2 triangles of this face
        for(int t = 0; t < 6; t++)
        {
            unsigned short idx = cubeIndices[faceStart + t];
            glm::vec3 pos = corners[idx];

            Vertex v;
            v.x  = pos.x;
            v.y  = pos.y;
            v.z  = pos.z;

            // Hard-edged face normal
            v.nx = faceNormal.x;
            v.ny = faceNormal.y;
            v.nz = faceNormal.z;

            // Very basic UV (optional)
            v.u = pos.x + 0.5f; 
            v.v = pos.y + 0.5f;

            finalVerts.push_back(v);
        }
    }

    for(auto v : finalVerts)
    {
        std::cout << "v normal: (" << v.nx << ", " << v.ny << ", " << v.nz << ")\n";
    }

    s_numIndices = (int)finalVerts.size();

    // Index buffer is just 0..35
    std::vector<unsigned short> finalIdx(s_numIndices);
    for(int i = 0; i < s_numIndices; i++)
        finalIdx[i] = (unsigned short) i;

    // Create GPU buffers
    s_vb = volpe::BufferManager::CreateVertexBuffer(
        finalVerts.data(), 
        (unsigned int)(finalVerts.size() * sizeof(Vertex))
    );
    s_ib = volpe::BufferManager::CreateIndexBuffer(
        finalIdx.data(), 
        (unsigned int) finalIdx.size()
    );

    // Create the declaration
    s_decl = new volpe::VertexDeclaration();
    s_decl->Begin();
        s_decl->SetVertexBuffer(s_vb);
        s_decl->SetIndexBuffer(s_ib);
        s_decl->AppendAttribute(volpe::AT_Position, 3, volpe::CT_Float);
        s_decl->AppendAttribute(volpe::AT_Normal,   3, volpe::CT_Float);
        s_decl->AppendAttribute(volpe::AT_TexCoord1,2, volpe::CT_Float);
    s_decl->End();

    s_inited = true;
}
*/





/*
void DebugCube::initGeometry()
{
    if(s_inited)
        return;

    // 8 corners of a unit cube from [-0.5..+0.5] in each axis.
    static const glm::vec3 corners[8] = {
        {-0.5f, -0.5f, -0.5f}, {+0.5f, -0.5f, -0.5f},
        {-0.5f, +0.5f, -0.5f}, {+0.5f, +0.5f, -0.5f},
        {-0.5f, -0.5f, +0.5f}, {+0.5f, -0.5f, +0.5f},
        {-0.5f, +0.5f, +0.5f}, {+0.5f, +0.5f, +0.5f}
    };

    // 6 faces, each with 2 triangles => 6 indices, total 36
    static const unsigned short cubeIndices[36] = {
        // Back face  (z = -0.5)
        0, 2, 1,   1, 2, 3,
        // Front face (z = +0.5)
        4, 5, 6,   5, 7, 6,
        // Left face  (x = -0.5)
        0, 4, 2,   2, 4, 6,
        // Right face (x = +0.5)
        1, 3, 5,   3, 7, 5,
        // Top face   (y = +0.5)
        2, 6, 3,   3, 6, 7,
        // Bottom face(y = -0.5)
        0, 1, 4,   1, 5, 4
    };

    std::vector<Vertex> finalVerts;
    finalVerts.reserve(36);

    // We'll iterate over each face in the index array, compute the face normal from the first triangle, 
    // then replicate that normal for all 6 vertices of that face.
    for(int f = 0; f < 6; ++f)
    {
        int faceStart = f * 6;  // each face has 6 indices => 2 triangles
        // Grab the first triangle (i0, i1, i2) from this face to compute the normal.
        unsigned short i0 = cubeIndices[faceStart + 0];
        unsigned short i1 = cubeIndices[faceStart + 1];
        unsigned short i2 = cubeIndices[faceStart + 2];

        glm::vec3 p0 = corners[i0];
        glm::vec3 p1 = corners[i1];
        glm::vec3 p2 = corners[i2];

        // IMPORTANT: The order of cross(...) arguments 
        // determines if the normal faces outward or inward.
        // We'll do cross((p1 - p0), (p2 - p0)) to match the winding 
        // in our index array. If your faces come out reversed, just swap them.
        glm::vec3 e1 = p1 - p0;
        glm::vec3 e2 = p2 - p0;
        glm::vec3 faceNormal = glm::normalize(glm::cross(e1, e2));

        // Now fill in 6 vertices for this face, each with the same faceNormal.
        for(int i = 0; i < 6; i++)
        {
            unsigned short idx = cubeIndices[faceStart + i];
            glm::vec3 pos = corners[idx];

            Vertex v;
            v.x = pos.x;
            v.y = pos.y;
            v.z = pos.z;
            v.nx = faceNormal.x;
            v.ny = faceNormal.y;
            v.nz = faceNormal.z;

            // Simple planar UV. 
            // (You can do more advanced UV if you want.)
            v.u = pos.x + 0.5f;
            v.v = pos.y + 0.5f;

            finalVerts.push_back(v);
        }
    }

    s_numIndices = (int)finalVerts.size();

    // The actual index buffer is just [0..N-1].
    std::vector<unsigned short> finalIdx;
    finalIdx.reserve(s_numIndices);
    for(int i = 0; i < s_numIndices; ++i)
        finalIdx.push_back((unsigned short)i);

    s_vb = volpe::BufferManager::CreateVertexBuffer(finalVerts.data(), (unsigned int)(finalVerts.size() * sizeof(Vertex)));
    s_ib = volpe::BufferManager::CreateIndexBuffer(finalIdx.data(), (unsigned int)finalIdx.size());

    s_decl = new volpe::VertexDeclaration();
    s_decl->Begin();
      s_decl->SetVertexBuffer(s_vb);
      s_decl->SetIndexBuffer(s_ib);
      s_decl->AppendAttribute(volpe::AT_Position, 3, volpe::CT_Float);
      s_decl->AppendAttribute(volpe::AT_Normal,   3, volpe::CT_Float);
      s_decl->AppendAttribute(volpe::AT_TexCoord1,2, volpe::CT_Float);
    s_decl->End();

    s_inited = true;
}
*/
/*
void DebugCube::initGeometry()
{
    if (s_inited) 
        return;

    // 8 corners of a unit cube centered at origin
    static const glm::vec3 corners[8] =
    {
        {-0.5f, -0.5f, -0.5f},
        {+0.5f, -0.5f, -0.5f},
        {-0.5f, +0.5f, -0.5f},
        {+0.5f, +0.5f, -0.5f},
        {-0.5f, -0.5f, +0.5f},
        {+0.5f, -0.5f, +0.5f},
        {-0.5f, +0.5f, +0.5f},
        {+0.5f, +0.5f, +0.5f}
    };

    // 6 faces × 2 triangles × 3 indices = 36
    static const unsigned short cubeIndices[36] =
    {
        // back face (z = -0.5)
        0,2,1,  1,2,3,
        // front face (z = +0.5)
        4,5,6,  5,7,6,
        // left face (x = -0.5)
        0,4,2,  2,4,6,
        // right face (x = +0.5)
        1,3,5,  3,7,5,
        // top face (y = +0.5)
        2,6,3,  3,6,7,
        // bottom face (y = -0.5)
        0,1,4,  1,5,4
    };

    // One normal per face, in the same order we grouped faces above:
    //   Face 0: back  => ( 0,  0, -1)
    //   Face 1: front => ( 0,  0, +1)
    //   Face 2: left  => (-1,  0,  0)
    //   Face 3: right => (+1,  0,  0)
    //   Face 4: top   => ( 0, +1,  0)
    //   Face 5: bottom=> ( 0, -1,  0)
    static const glm::vec3 faceNormals[6] =
    {
        { 0.0f,  0.0f, -1.0f}, // back
        { 0.0f,  0.0f, +1.0f}, // front
        {-1.0f,  0.0f,  0.0f}, // left
        {+1.0f,  0.0f,  0.0f}, // right
        { 0.0f, +1.0f,  0.0f}, // top
        { 0.0f, -1.0f,  0.0f}  // bottom
    };

    std::vector<Vertex> finalVerts;
    finalVerts.reserve(36);

    // Each of the 6 faces has 6 indices in the array
    for(int face = 0; face < 6; face++)
    {
        glm::vec3 fn = faceNormals[face];

        // For this face, pick its 6 indices
        int startIndex = face * 6;
        for(int i = 0; i < 6; i++)
        {
            unsigned short idx = cubeIndices[startIndex + i];
            const glm::vec3& pos = corners[idx];

            Vertex v;
            v.x  = pos.x;
            v.y  = pos.y;
            v.z  = pos.z;

            // Hard-edged normal:
            v.nx = fn.x;
            v.ny = fn.y;
            v.nz = fn.z;

            // Simple UV if needed
            v.u = (i < 3) ? 0.0f : 1.0f;
            v.v = (i == 1 || i == 2 || i == 5) ? 1.0f : 0.0f;

            finalVerts.push_back(v);
        }
    }

    // We simply use [0..35] as indices (one unique vertex per index).
    s_numIndices = (int) finalVerts.size();
    std::vector<unsigned short> finalIdx(s_numIndices);
    for(int i = 0; i < s_numIndices; i++)
        finalIdx[i] = (unsigned short)i;

    // Create buffers
    s_vb = volpe::BufferManager::CreateVertexBuffer(finalVerts.data(),
                  (unsigned int) (finalVerts.size()*sizeof(Vertex)));
    s_ib = volpe::BufferManager::CreateIndexBuffer(finalIdx.data(), 
                  (unsigned int) finalIdx.size());

    // Create declaration
    s_decl = new volpe::VertexDeclaration();
    s_decl->Begin();
      s_decl->SetVertexBuffer(s_vb);
      s_decl->SetIndexBuffer(s_ib);
      s_decl->AppendAttribute(volpe::AT_Position, 3, volpe::CT_Float);
      s_decl->AppendAttribute(volpe::AT_Normal,   3, volpe::CT_Float);
      s_decl->AppendAttribute(volpe::AT_TexCoord1,2, volpe::CT_Float);
    s_decl->End();

    s_inited = true;
}
*/
// /*
void DebugCube::initGeometry()
{
    if(s_inited)
        return;

    static const glm::vec3 corners[8] = {
        {-0.5f,-0.5f,-0.5f}, {+0.5f,-0.5f,-0.5f}, 
        {-0.5f,+0.5f,-0.5f}, {+0.5f,+0.5f,-0.5f}, 
        {-0.5f,-0.5f,+0.5f}, {+0.5f,-0.5f,+0.5f}, 
        {-0.5f,+0.5f,+0.5f}, {+0.5f,+0.5f,+0.5f}
    };

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

    static const glm::vec3 faceNormals[6] =
    {
        { 0.0f,  0.0f, -1.0f}, // back
        { 0.0f,  0.0f, +1.0f}, // front
        {-1.0f,  0.0f,  0.0f}, // left
        {+1.0f,  0.0f,  0.0f}, // right
        { 0.0f, +1.0f,  0.0f}, // top
        { 0.0f, -1.0f,  0.0f}  // bottom
    };


    std::vector<Vertex> finalVerts; 
    finalVerts.reserve(36);
    for(int f=0; f<6; f++)
    {

        int faceStart = f*6;
        
        unsigned short i0 = cubeIndices[faceStart + 0];
        unsigned short i1 = cubeIndices[faceStart + 1];
        unsigned short i2 = cubeIndices[faceStart + 2];

        glm::vec3 p0 = corners[i0];
        glm::vec3 p1 = corners[i1];
        glm::vec3 p2 = corners[i2];
        // glm::vec3 faceNormal = glm::normalize(glm::cross(p1-p0, p2-p0));
        // glm::vec3 faceNormal = glm::normalize(glm::cross(p2 - p0, p1 - p0));
        
        glm::vec3 e1 = p1 - p0;
        glm::vec3 e2 = p2 - p0;
        glm::vec3 faceNormal = glm::normalize(glm::cross(e1, e2));
        
        glm::vec3 fn = faceNormals[f];

        for(int i=0; i<6; i++)
        {
            unsigned short idx = cubeIndices[faceStart + i];
            glm::vec3 pos = corners[idx];
            Vertex v;
            v.x = pos.x; 
            v.y = pos.y;
            v.z = pos.z;
            
            v.nx = fn.x;
            v.ny = fn.y;
            v.nz = fn.z;

            // v.nx = faceNormal.x;
            // v.ny = faceNormal.y;
            // v.nz = faceNormal.z;
            
            v.u = (faceNormal.x != 0.f) ? (pos.y + 0.5f) : (pos.x + 0.5f); 
            v.v = (faceNormal.z != 0.f) ? (pos.y + 0.5f) : (pos.z + 0.5f);
            finalVerts.push_back(v);
        }
    }

    s_numIndices = (int)finalVerts.size();

    std::vector<unsigned short> finalIdx;
    finalIdx.reserve(s_numIndices);
    for(int i=0; i<s_numIndices; i++)
        finalIdx.push_back((unsigned short)i);

    s_vb = volpe::BufferManager::CreateVertexBuffer(finalVerts.data(), (unsigned int) (finalVerts.size()*sizeof(Vertex)));
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
// */