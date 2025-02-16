#include "DebugSphere.h"
#include "../volpe/Volpe.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <vector>

// Static members
bool DebugSphere::s_inited               = false;
volpe::VertexBuffer* DebugSphere::s_vb   = nullptr;
volpe::IndexBuffer*  DebugSphere::s_ib   = nullptr;
volpe::VertexDeclaration* DebugSphere::s_decl = nullptr;
int DebugSphere::s_numIndices            = 0;

///////////////////////////////////////////////
// Constructor / Destructor
///////////////////////////////////////////////
DebugSphere::DebugSphere(const std::string& name, float radius)
: Node(name), m_color(1.f,1.f,1.f), m_radius(radius), m_boundingVolumeSphere(nullptr)
{
    // 1) Create bounding volume
    m_boundingVolumeSphere = new SphereVolume(glm::vec3(0.0f), m_radius);
    SetBoundingVolume(m_boundingVolumeSphere);

    // 2) Create the material
    volpe::Material* mat = volpe::MaterialManager::CreateMaterial("DebugSphereMat");
    mat->SetProgram("data/Unlit3d.vsh", "data/Unlit3d.fsh");
    mat->SetDepthTest(true);
    mat->SetDepthWrite(true);
    SetMaterial(mat);

    // 3) Build static geometry if needed
    initGeometry(20, 20);
}

DebugSphere::~DebugSphere()
{
    // destroy material
    if(GetMaterial()) {
        volpe::MaterialManager::DestroyMaterial(GetMaterial());
        SetMaterial(nullptr);
    }
}

///////////////////////////////////////////////
// Public
///////////////////////////////////////////////
void DebugSphere::setColor(GLubyte r, GLubyte g, GLubyte b)
{
    m_color = glm::vec3(r/255.f, g/255.f, b/255.f);
}

void DebugSphere::setRadius(float r)
{
    m_radius = r;
    if(m_boundingVolumeSphere) {
        m_boundingVolumeSphere->radius = r;
    }
}

void DebugSphere::draw(const glm::mat4& proj, const glm::mat4& view, bool skipBind)
{
    // 1) Actually render the sphere
    Render(proj, view);

    // 2) draw bounding volume lines
    if(GetBoundingVolume()) {
        GetBoundingVolume()->DrawMe(proj, view);
    }

    // 3) node children
    Node::draw(proj, view, skipBind);
}

///////////////////////////////////////////////
// Private
///////////////////////////////////////////////
void DebugSphere::Render(const glm::mat4& proj, const glm::mat4& view)
{
    if(!s_inited || s_numIndices<=0) 
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
    mat->SetUniform("u_color",    m_color);
    mat->Apply();

    s_decl->Bind();
    glDrawElements(GL_TRIANGLES, s_numIndices, GL_UNSIGNED_SHORT, 0);
}

///////////////////////////////////////////////
// static method: create geometry if not yet
///////////////////////////////////////////////
void DebugSphere::initGeometry(int sectorCount, int stackCount)
{
    if(s_inited)
        return;

    // We'll build a typical sphere with "sectorCount" slices around 
    // the equator, and "stackCount" from top to bottom.

    struct SphereVtx {
        float x,y,z;
        float nx,ny,nz;
        float u,v;
    };

    std::vector<SphereVtx> verts;
    std::vector<unsigned short> idx;

    // float PI = 3.14159f;
    float sectorStep = 2.f * PI / sectorCount;
    float stackStep  = PI / stackCount;
    for(int i=0; i<=stackCount; i++)
    {
        float stackAngle = PI/2.f - i*stackStep; // from +PI/2 down to -PI/2
        float xy = cosf(stackAngle);
        float z  = sinf(stackAngle);

        for(int j=0; j<=sectorCount; j++)
        {
            float sectorAngle = j*sectorStep;
            float x = xy*cosf(sectorAngle);
            float y = xy*sinf(sectorAngle);

            SphereVtx v;
            // position (unit sphere for now)
            v.x = x;
            v.y = y;
            v.z = z;
            // normal is the same as position for a unit sphere
            v.nx = x;
            v.ny = y;
            v.nz = z;
            // uv
            v.u  = (float)j / sectorCount;
            v.v  = (float)i / stackCount;

            verts.push_back(v);
        }
    }

    // generate indices
    for(int i=0; i<stackCount; i++)
    {
        int k1 = i*(sectorCount+1);
        int k2 = k1 + sectorCount+1;
        for(int j=0; j<sectorCount; j++, k1++, k2++)
        {
            if(i!=0) {
                idx.push_back((unsigned short)k1);
                idx.push_back((unsigned short)k2);
                idx.push_back((unsigned short)(k1+1));
            }
            if(i!=(stackCount-1)) {
                idx.push_back((unsigned short)(k1+1));
                idx.push_back((unsigned short)k2);
                idx.push_back((unsigned short)(k2+1));
            }
        }
    }
    s_numIndices = (int) idx.size();

    // Now create the GPU buffers
    s_vb = volpe::BufferManager::CreateVertexBuffer(verts.data(), (unsigned int)(sizeof(SphereVtx)*verts.size()));
    s_ib = volpe::BufferManager::CreateIndexBuffer(idx.data(), (unsigned int)idx.size());

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
