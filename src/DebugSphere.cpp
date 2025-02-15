#include "DebugSphere.h"
#include "iostream"
#include <vector>
#include <glm/gtx/norm.hpp>

// static const float PI = 3.14159265358979323846f;
static const float BASE_RADIUS = 0.5f; // base radius used for vertex generation

DebugSphere::DebugSphere(const std::string& name, float radius)
    : Node(name), m_radius(radius)
{
    // Create a bounding volume as a SphereVolume centered at (0,0,0)
    // with the given radius.
    m_boundingVolumeSphere = new SphereVolume(glm::vec3(0.0f), m_radius);
    m_boundingVolume = m_boundingVolumeSphere; // store in the Node base

    // Create a shader program.
    m_pProgram = volpe::ProgramManager::CreateProgram("data/Unlit3d.vsh", "data/Unlit3d.fsh");
    // Optionally, enable depth test and write:
    // m_pProgram->SetDepthTest(true);
    // m_pProgram->SetDepthWrite(true);

    // Generate the sphere geometry.
    genVertexData();
}

DebugSphere::~DebugSphere() {
    // Note: Depending on your engine, you might need to destroy the shader program.
    // volpe::MaterialManager::DestroyMaterial(m_pProgram);
    if(m_vertexBuffer)
        volpe::BufferManager::DestroyBuffer(m_vertexBuffer);
    if(m_vertexDecl)
        delete m_vertexDecl;
    if(m_boundingVolumeSphere)
        delete m_boundingVolumeSphere;
}

void DebugSphere::pushVertexData(volpe::VertexBuffer*& vBuffer, volpe::VertexDeclaration*& vDecl, const std::vector<Vertex>& inVerts) {
    if(vBuffer) {
        volpe::BufferManager::DestroyBuffer(vBuffer);
        vBuffer = nullptr;
    }
    if(vDecl) {
        delete vDecl;
        vDecl = nullptr;
    }
    if(inVerts.empty()) {
        m_numIndices = 0;
        return;
    }
    vBuffer = volpe::BufferManager::CreateVertexBuffer(inVerts.data(), inVerts.size() * sizeof(Vertex));
    vDecl = new volpe::VertexDeclaration();
    vDecl->Begin();
    vDecl->AppendAttribute(volpe::AT_Position,  3, volpe::CT_Float);
    vDecl->AppendAttribute(volpe::AT_Color,     4, volpe::CT_UByte);
    vDecl->AppendAttribute(volpe::AT_TexCoord1, 2, volpe::CT_Float);
    vDecl->AppendAttribute(volpe::AT_Normal,    3, volpe::CT_Float);
    vDecl->SetVertexBuffer(vBuffer);
    vDecl->End();
}

void DebugSphere::_genVerts(int sectorCount, int stackCount) {
    Vertex v;
    std::vector<Vertex> verts;
    std::vector<unsigned short> indices;

    float x, y, z, xy;
    float nx, ny, nz;
    float lengthInv = 1.0f / m_radius;
    float s, t;
    float sectorStep = 2 * PI / sectorCount;
    float stackStep = PI / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i) {
        stackAngle = PI / 2 - i * stackStep;  // from pi/2 to -pi/2
        xy = m_radius * cosf(stackAngle);      // r * cos(u)
        z = m_radius * sinf(stackAngle);       // r * sin(u)

        for (int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;
            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);

            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            

            s = (float)j / sectorCount;
            t = (float)i / stackCount;

            v.x = x; v.y = y; v.z = z;
            v.nx = nx; v.ny = ny; v.nz = nz;
            v.u = s; v.v = t;
            
            v.r = r;
            v.g = g;
            v.b = b;
            v.a = 255;

            verts.push_back(v);
        }
    }

    int k1, k2;
    for (int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;
        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
    m_numIndices = (int)indices.size();

    // Instead of manually creating buffers then calling pushVertexData,
    // simply call pushVertexData with the generated vertex list.
    pushVertexData(m_vertexBuffer, m_vertexDecl, verts);

    // Create an index buffer and attach it to the vertex declaration.
    volpe::IndexBuffer* indexBuffer = volpe::BufferManager::CreateIndexBuffer(indices.data(), m_numIndices);
    m_vertexDecl->Begin();
    m_vertexDecl->SetIndexBuffer(indexBuffer);
    m_vertexDecl->End();
}

void DebugSphere::genVertexData() {
    // Generate sphere geometry with 20 sectors and 20 stacks.
    _genVerts(20, 20);
}

void DebugSphere::Render(const glm::mat4& proj, const glm::mat4& view, bool skipBind) {
    if (!m_pProgram || !m_vertexBuffer || !m_vertexDecl || m_numIndices == 0) {
        std::cerr << "Sphere Render Skipped\n";
        return;
    }
    glm::mat4 world = getWorldTransform();
    glm::mat4 worldIT = glm::transpose(glm::inverse(world));
    
    if (!skipBind)
        m_pProgram->Bind();

    m_pProgram->Bind();
    m_pProgram->SetUniform("projection", proj);
    m_pProgram->SetUniform("view", view);
    m_pProgram->SetUniform("world", world);
    m_pProgram->SetUniform("worldIT", worldIT);
    // m_pProgram->SetUniform("u_color", m_color);
    m_vertexDecl->Bind();
    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_SHORT, 0);
}

void DebugSphere::draw(const glm::mat4& proj, const glm::mat4& view, bool skipBind) {
    Render(proj, view, skipBind);
    Node::draw(proj, view);
}

void DebugSphere::DrawBoundingVolume(const glm::mat4& proj, const glm::mat4& view) {
    // return;
    // Draw a wireframe circle representing the sphere's boundary in the XZ plane.
    glm::vec3 center = glm::vec3(getWorldTransform()[3]);
    int segments = 32;
    float angleStep = 2.0f * PI / segments;
    glm::vec3 color(0.0f, 1.0f, 0.0f);
    for (int i = 0; i < segments; ++i) {
        float a = i * angleStep;
        float b = (i + 1) * angleStep;
        glm::vec3 p1 = center + glm::vec3(m_radius * cos(a), 0, m_radius * sin(a));
        glm::vec3 p2 = center + glm::vec3(m_radius * cos(b), 0, m_radius * sin(b));
        DebugRender::Instance().DrawLine(p1, p2, color, "BoundingVolumes");
    }
}
