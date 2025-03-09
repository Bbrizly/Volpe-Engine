#include "DebugSphere.h"
#include "../volpe/Volpe.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <vector>
#include "Vertex.h"

// Static members
bool DebugSphere::s_inited               = false;
volpe::VertexBuffer* DebugSphere::s_vb   = nullptr;
volpe::IndexBuffer*  DebugSphere::s_ib   = nullptr;
volpe::VertexDeclaration* DebugSphere::s_decl = nullptr;
int DebugSphere::s_numIndices            = 0;


DebugSphere::DebugSphere(const std::string& name, float radius)
: Node(name), m_color(1.f,1.f,1.f), m_radius(radius), m_boundingVolumeSphere(nullptr)
{
    m_boundingVolumeSphere = new SphereVolume(glm::vec3(0.0f), m_radius);
    SetBoundingVolume(m_boundingVolumeSphere);

    volpe::Material* mat = volpe::MaterialManager::CreateMaterial("DebugSphereMat");
    // volpe::Material* mat = volpe::MaterialManager::CreateMaterial(name + "DebugSphereMat"); //to work for multiple particle systems.
    mat->SetProgram("data/Unlit3d.vsh", "data/Unlit3d.fsh");
    mat->SetDepthTest(true);
    mat->SetDepthWrite(true);
    SetMaterial(mat);
    m_ownsMaterial = true;

    initGeometry(10, 10);
}

DebugSphere::~DebugSphere()
{
    // if(GetMaterial()) {
    //     volpe::MaterialManager::DestroyMaterial(GetMaterial());
    //     SetMaterial(nullptr);
    // }
}

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

void DebugSphere::draw(const glm::mat4& proj, const glm::mat4& view)
{
    Render(proj, view);

    Node::draw(proj, view);
}

void DebugSphere::Render(const glm::mat4& proj, const glm::mat4& view)
{
    if(!s_inited || s_numIndices<=0) 
        return;
    volpe::Material* mat = GetMaterial();
    if(!mat)
        return;

    glm::mat4 world = getWorldTransform();
    world = world * glm::scale(glm::mat4(1.0f), glm::vec3(m_radius));
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

void DebugSphere::initGeometry(int sectorCount, int stackCount)
{
    if(s_inited)
        return;

    std::vector<Vertex> verts;
    std::vector<unsigned short> idx;

    float sectorStep = 2.f * PI / sectorCount;
    float stackStep  = PI / stackCount;
    for(int i=0; i<=stackCount; i++)
    {
        float stackAngle = PI/2.f - i*stackStep;
        float xy = cosf(stackAngle);
        float z  = sinf(stackAngle);

        for(int j=0; j<=sectorCount; j++)
        {
            float sectorAngle = j*sectorStep;
            float x = xy*cosf(sectorAngle);
            float y = xy*sinf(sectorAngle);

            Vertex v;
            
            v.x = x;
            v.y = y;
            v.z = z;
            
            glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
            v.nx = normal.x;
            v.ny = normal.y;
            v.nz = normal.z;

            // v.nx = x;
            // v.ny = y;
            // v.nz = z;
            
            v.u  = (float)j / sectorCount;
            v.v  = (float)i / stackCount;

            verts.push_back(v);
        }
    }

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

    s_vb = volpe::BufferManager::CreateVertexBuffer(verts.data(), (unsigned int)(sizeof(Vertex)*verts.size()));
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
