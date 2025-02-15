#include "Sphere.h"
#include "iostream"

volpe::VertexBuffer* Sphere::s_pVB = nullptr;
volpe::IndexBuffer* Sphere::s_pIB = nullptr;
volpe::VertexDeclaration* Sphere::s_pDecl = nullptr;
int Sphere::s_numIndices = 0;

struct SphereVertex
{
    float x,y,z;
    float nx,ny,nz;
    float u,v;
};

Sphere::Sphere(float radius)
{
    SetRadius(radius);

    m_pMat = volpe::MaterialManager::CreateMaterial("__sphereMat");
    // m_pMat->SetProgram("samplefw/data/sphere.vsh", "samplefw/data/sphere.fsh");
    // m_pMat->SetProgram("data/Unlit3d.vsh", "data/Unlit3d.fsh");
    m_pMat->SetProgram("data/PointLights.vsh","data/PointLights.fsh");
    m_pMat->SetDepthTest(true);
    m_pMat->SetDepthWrite(true);

    if(!s_pVB)
    {
        _genVerts(20,20);
    }
}

Sphere::~Sphere()
{
    volpe::MaterialManager::DestroyMaterial(m_pMat);
}

void Sphere::Render(const glm::mat4& worldMatrix, const glm::mat4& viewMatrix, const glm::mat4& projMatrix)
{
    glm::mat4 world = worldMatrix * glm::translate(glm::mat4(1.0f), m_pos) * glm::scale(glm::mat4(1.0f), m_scale);
    m_pMat->SetUniform("projection", projMatrix);
    m_pMat->SetUniform("view", viewMatrix);
    m_pMat->SetUniform("world", world);
    m_pMat->SetUniform("worldIT", glm::transpose(glm::inverse(world)));
    m_pMat->SetUniform("u_color", m_color);
    m_pMat->Apply();

    s_pDecl->Bind();
    glDrawElements(GL_TRIANGLES, s_numIndices, GL_UNSIGNED_SHORT, 0);
}

void Sphere::_genVerts(int sectorCount, int stackCount)
{
    SphereVertex v;
    std::vector<SphereVertex> verts;
    std::vector<unsigned short> indices;

    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / m_radius;    // vertex normal
    float s, t;                                     // vertex texCoord

    float sectorStep = 2 * PI / sectorCount;
    float stackStep = PI / stackCount;
    float sectorAngle, stackAngle;

    for(int i = 0; i <= stackCount; ++i)
    {
        stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = BASE_RADIUS * cosf(stackAngle);             // r * cos(u)
        z = BASE_RADIUS * sinf(stackAngle);              // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for(int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;

            // vertex tex coord (s, t) range between [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;

            v.x = x;
            v.y = y;
            v.z = z;
            v.nx = nx;
            v.ny = ny;
            v.nz = nz;
            v.u = s;
            v.v = t;

            verts.push_back(v);
        }
    }

    int k1, k2;
    for(int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for(int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if(i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            // k1+1 => k2 => k2+1
            if(i != (stackCount-1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    s_numIndices = (int) indices.size();

    s_pVB = volpe::BufferManager::CreateVertexBuffer(verts.data(), (unsigned int) (sizeof(SphereVertex) * verts.size()));
    s_pIB = volpe::BufferManager::CreateIndexBuffer(indices.data(), s_numIndices);

    s_pDecl = new volpe::VertexDeclaration();
    s_pDecl->Begin();
    s_pDecl->SetVertexBuffer(s_pVB);
    s_pDecl->SetIndexBuffer(s_pIB);
    s_pDecl->AppendAttribute(volpe::AT_Position, 3, volpe::CT_Float);
    s_pDecl->AppendAttribute(volpe::AT_Normal, 3, volpe::CT_Float);
    s_pDecl->AppendAttribute(volpe::AT_TexCoord1, 2, volpe::CT_Float); //uv
    s_pDecl->End();
}