#include "ParticleNode.h"
#include "SphereVolume.h"

ParticleNode::ParticleNode(const std::string& name)
    : Node(name),
    m_randGen(std::random_device{}()),
    m_distPos(minDist, maxDist),
    m_distVel(minSpeed, maxSpeed),
    m_distLife(minLife, maxLife)
{
    m_material = volpe::MaterialManager::CreateMaterial("ParticleMat");
    m_material->SetProgram("data/Unlit3d.vsh", "data/Unlit3d.fsh");
    m_material->SetDepthTest(true);
    m_material->SetDepthWrite(false); 
    m_material->SetBlend(true);

    SphereVolume* vol = new SphereVolume(glm::vec3(0), 10.0f);
    SetBoundingVolume(vol);

    spawnParticles(goalAmount);
}

ParticleNode::~ParticleNode()
{
    if(m_vb) {
        volpe::BufferManager::DestroyBuffer(m_vb);
        m_vb = nullptr;
    }
    if(m_decl) {
        delete m_decl;
        m_decl = nullptr;
    }
    if(m_material) {
        volpe::MaterialManager::DestroyMaterial(m_material);
        m_material = nullptr;
    }
}

void ParticleNode::spawnParticles(int count)
{
    // m_particles.clear();
    for(int i = 0; i < count; ++i) {
        Particle p;
        p.position = glm::vec3(m_distPos(m_randGen), m_distPos(m_randGen), m_distPos(m_randGen));
        p.velocity = glm::vec3(m_distVel(m_randGen)*1.0f,
                               m_distVel(m_randGen)*1.0f,
                               m_distVel(m_randGen)*1.0f);
        p.maxLife  = m_distLife(m_randGen);
        p.life     = p.maxLife;
        m_particles.push_back(p);
    }
}

void ParticleNode::update(float dt)
{
    Node::update(dt);

    for(auto& p : m_particles)
    {
        p.position += p.velocity * dt;
        p.life -= dt;

        p.velocity *= 0.99f; //make them get slower over time
    }

    // Remove dead shits
    m_particles.erase(
       std::remove_if(m_particles.begin(), m_particles.end(), 
           [](const Particle& p){ return (p.life <= 0.0f); }),
       m_particles.end()
    );
    
    // if less than goalamount parcles spawn new ones
    if((int)m_particles.size() < goalAmount) {
        spawnParticles(goalAmount - (int)m_particles.size());
    }
}

void ParticleNode::draw(const glm::mat4& proj, const glm::mat4& view)
{
    // Render self
    if(!m_material) return;

    buildVertexData(view);

    if(!m_vb || !m_decl) return;

    m_material->SetUniform("projection", proj);
    m_material->SetUniform("view",       view);

    glm::mat4 world(1.0f);
    glm::mat4 worldIT = glm::transpose(glm::inverse(world));
    m_material->SetUniform("world",      world);
    m_material->SetUniform("worldIT",    worldIT);

    m_material->SetUniform("u_color", glm::vec3(1,1,1));

    m_material->Apply();
    m_decl->Bind();

    int vertexCount = (int)m_particles.size() * 6;
    if(vertexCount > 0) {
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }

    Node::draw(proj, view);
}

void ParticleNode::buildVertexData(const glm::mat4& view)
{
    glm::vec3 right, up;
    getCameraRightUp(view, right, up);

    float halfSize = 0.25f; 

    std::vector<QuadVertex> vertexData;
    vertexData.reserve(m_particles.size() * 6);

    for(const auto& p : m_particles)
    {
        glm::vec3 offsetR = right * halfSize;
        glm::vec3 offsetU = up    * halfSize;

        // bottom-left
        QuadVertex v0;
        v0.x = p.position.x - offsetR.x - offsetU.x;
        v0.y = p.position.y - offsetR.y - offsetU.y;
        v0.z = p.position.z - offsetR.z - offsetU.z;
        v0.u = 0.0f; v0.v = 0.0f;

        // bottom-right
        QuadVertex v1;
        v1.x = p.position.x + offsetR.x - offsetU.x;
        v1.y = p.position.y + offsetR.y - offsetU.y;
        v1.z = p.position.z + offsetR.z - offsetU.z;
        v1.u = 1.0f; v1.v = 0.0f;

        // top-left
        QuadVertex v2;
        v2.x = p.position.x - offsetR.x + offsetU.x;
        v2.y = p.position.y - offsetR.y + offsetU.y;
        v2.z = p.position.z - offsetR.z + offsetU.z;
        v2.u = 0.0f; v2.v = 1.0f;

        // top-right
        QuadVertex v3;
        v3.x = p.position.x + offsetR.x + offsetU.x;
        v3.y = p.position.y + offsetR.y + offsetU.y;
        v3.z = p.position.z + offsetR.z + offsetU.z;
        v3.u = 1.0f; v3.v = 1.0f;

        vertexData.push_back(v0);
        vertexData.push_back(v1);
        vertexData.push_back(v2);
        
        vertexData.push_back(v2);
        vertexData.push_back(v1);
        vertexData.push_back(v3);
    }

    GLsizeiptr dataSize = vertexData.size() * sizeof(QuadVertex);
    if(!m_vb) {
        m_vb = volpe::BufferManager::CreateVertexBuffer(vertexData.data(), (unsigned int)dataSize);

        m_decl = new volpe::VertexDeclaration();
        m_decl->Begin();
          m_decl->SetVertexBuffer(m_vb);

          m_decl->AppendAttribute(volpe::AT_Position, 3, volpe::CT_Float);
          m_decl->AppendAttribute(volpe::AT_TexCoord1,2, volpe::CT_Float);
        m_decl->End();
    }
    else {
        volpe::BufferManager::UpdateVertexBuffer(m_vb, vertexData.data(), (unsigned int)dataSize);
    }
}

void ParticleNode::getCameraRightUp(const glm::mat4& view, glm::vec3& outRight, glm::vec3& outUp)
{
    glm::mat3 rot = glm::inverse(glm::mat3(view)); 
    outRight = glm::normalize(glm::vec3(rot[0][0], rot[0][1], rot[0][2]));
    outUp    = glm::normalize(glm::vec3(rot[1][0], rot[1][1], rot[1][2]));
}
