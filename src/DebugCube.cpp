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

DebugCube::DebugCube(const std::string& name)
: Node(name)
{
    m_boundingSphere.center = glm::vec3(0,0,0);
    m_boundingSphere.radius = 0.8f; // Smaller than node
    
    m_pProgram = volpe::ProgramManager::CreateProgram("data/Unlit3d.vsh", "data/Unlit3d.fsh");
    // m_pProgram = volpe::ProgramManager::CreateProgram("data/Directional3D.vsh", "data/Directional3D.fsh");

    genVertexData();
}
DebugCube::~DebugCube(){}

void DebugCube::genVertexData() {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> rgb(0.0f, 255.0f);
    GLubyte r = rgb(gen) //255
           ,g = rgb(gen) //255
           ,b = rgb(gen) //255
           ;

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

AABB3D DebugCube::getWorldAABB3D() const
{
    AABB3D box;
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

    std::cout << "DebugCube created with " << m_numVertices << " vertices.\n";
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

    // if(m_numLightsAffecting >= 1)
    // {
    //     m_pProgram->SetUniform("u_pointLights", m_lights[0].getLightPosition);
    //     m_pProgram->SetUniform("u_lightCount", m_numLightsAffecting);
    // }

    //IF affected by light: Set Uniform Light info (list of lights & amount etc)

    // m_pProgram->SetUniform("u_texture", 0);

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
